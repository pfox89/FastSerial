#pragma once

#if defined(_WIN32)
#  if defined(EXPORTING_SERIALENUM)
#    define DECLSPEC __declspec(dllexport)
#  elif defined(SERIAL_STATIC)
#    define DECLSPEC
#  else
#    define DECLSPEC __declspec(dllimport)
#  endif
#else // non windows
#  define DECLSPEC [[gnu::visibility("default")]]
#endif

#ifdef __cplusplus 
/// Possible types of bus this device can be connected to
enum class SerialBusType
#else
enum SerialBusType
#endif
{
  BUS_UNKNOWN  = 0,
  BUS_USB      = 0x1,
  BUS_PCI      = 0x2,
  BUS_PNP      = 0x4,
  BUS_PLATFORM = 0x8,
  BUS_ANY      = BUS_USB | BUS_PCI | BUS_PNP | BUS_PLATFORM
};
#ifdef __cplusplus 
/// Possible types of bus this device can be connected to
enum class LocationType
#else
enum LocationType
#endif
{
  NONE,
  PCI_ROOT,
  PCI_SLOT,
  PNP_ROOT,
  PNP_SLOT,
  USB_ROOT,
  USB_PORT
};

/// Node in serial device path to allow iterating over path
struct SerialDevicePathNode
{
  const char*    path;
  LocationType   type;
  unsigned short number;
};

/// Information about device
struct SerialDeviceInfo
{
  /// Logical device path (e.g. interface)
  const char* lpath;
  /// Physical device path
  const char* path;
  /// Device name
  const char* name;
  /// Device description
  const char* description;
  /// Device manufacturer string
  const char* manufacturer;
  /// Numeric vendor id (VID on USB, VEN on PCI)
  unsigned short vid;
  /// Numeric device id (PID on USB, DEV on PCI)
  unsigned short pid;
  /// Type of bus device is connected to
  SerialBusType type;

};

#ifdef __cplusplus 

#include <iosfwd>
#include <string>

extern "C" {
#endif

  /// Start enumerating devices
  /// \returns Status/error code of system_category type
  /// \remarks Always call SerialEnum_Finish after this function to free resources allocated
  DECLSPEC int SerialEnum_StartEnumeration(unsigned int typeMask);

  /// Gets info for next device
  /// \param[out] info Pointer to info struct to receive device info
  /// \returns Status/error code of system_category type
  /// \remark Strings remain valid only until another SerialEnum function is called, so they should be copied
  DECLSPEC int SerialEnum_Next(SerialDeviceInfo* info);

  /// Finish enumeration, freeing all associated resources
  DECLSPEC void SerialEnum_Finish();

  DECLSPEC int SerialEnum_PathTokNext(SerialDevicePathNode* path);

  /// @brief Get null terminated string representing location type
  DECLSPEC const char* SerialEnum_LocationType_to_cstring(LocationType type);

  /// \brief Print device path to string
  /// \param[in]  port   Port to print device path for
  /// \param[out] buffer Destination to print string to
  /// \param[in]  size   Size of buffer
  /// \return Number of bytes printed 
  DECLSPEC int SerialDevicePathNode_Path_print(const SerialDeviceInfo& port, char* buffer, int size);

  /// Get String describing SerialBusType
  DECLSPEC const char* SerialBusType_to_cstring(SerialBusType type);

#ifdef __cplusplus 
}

/// Stream insertion operator to print SerialBusType
DECLSPEC std::ostream& operator<<(std::ostream& os, SerialBusType type) noexcept;

/// Stream insertion operator to pretty-print serial device information
DECLSPEC std::ostream& operator<<(std::ostream& os, const SerialDeviceInfo& port) noexcept;

/// Stream insertion operator to pretty-print serial device node
DECLSPEC std::ostream& operator<<(std::ostream& os, const SerialDevicePathNode& node) noexcept;

/// C++ overload to convert to SerialBusType to cstring
static inline const char* to_cstring(SerialBusType type) noexcept
{
  return SerialBusType_to_cstring(type);
}

/// Convert SerialBusType to C++ string
inline std::string to_string(SerialBusType type) noexcept
{
  return std::string(SerialBusType_to_cstring(type));
}
#endif

#undef DECLSPEC

#ifdef __cplusplus

namespace Serial
{


#ifdef __cplusplus
  /// @brief Range of serial device path nodes to enable iterating over path
  struct Path
  {
    SerialDevicePathNode node;

    Path(const SerialDeviceInfo& info)
      : node{ info.path, LocationType::NONE, 0}
    {}

    struct iterator
    {
      Path& path;

      iterator& operator++() noexcept
      {
        SerialEnum_PathTokNext(&path.node);
        return *this;
      }
     
      const SerialDevicePathNode& operator*() const noexcept { return path.node; }
      const SerialDevicePathNode* operator->() const noexcept { return &path.node; }

      bool operator!=(iterator&)
      {
        return path.node.path != nullptr;
      }
    };

    iterator begin() noexcept { return ++iterator{ *this }; }
    iterator end() noexcept { return iterator{ *this }; }
  };
#endif

  /// Iterator to iterate over ports enumerator
  struct PortIter
  {
    // Create iterator over ports using type mask
    PortIter(uint32_t typeMask) noexcept
      : _status(SerialEnum_StartEnumeration(typeMask)), _info{}
    {
      // Populate info immediately
      ++(*this);
    }

    /// Passing an int constructs an dummy "end" iterator by setting status to invalid value
    PortIter(int) noexcept
      : _status(INT32_MAX), _info{}
    {}

    /// Don't allow copying iterator, ownership must be retained
    PortIter(const PortIter&) = delete;

    /// Move iterator takes ownership
    PortIter(PortIter&& other) noexcept
      : _status(other._status), _info(other._info)
    {
      other._status = INT32_MAX;
    }

    /// Incrementing iterator gets next device
    PortIter& operator++() noexcept
    {
      if (_status == 0)
      {
        _status = SerialEnum_Next(&_info);
      }
     
      return *this;
    }

    const SerialDeviceInfo& operator*() const noexcept
    {
      return _info;
    }

    const SerialDeviceInfo* operator->() const noexcept
    {
      return &_info;
    }

    bool operator!=(const PortIter&) const noexcept
    {
      return (_status == 0);
    }

    /// Destructor frees resources
    ~PortIter() noexcept
    {
      if (_status != INT32_MAX)
        SerialEnum_Finish();
    }
  private:  
    int32_t          _status;
    SerialDeviceInfo _info;

  };

  /// Constexpr range template to allow enumerator to be used as a range
  template<SerialBusType type>
  struct PortInfo
  {
    PortIter begin() const noexcept
    {
      return PortIter(static_cast<unsigned>(type));
    }
    PortIter end() const noexcept
    {
      return PortIter(INT32_MAX);
    }
  };

  /// \brief Enuemrator object for all ports
  /// Use begin() and end() functions to get iterators to port range, 
  /// or use in range-based for loop, e.g. `for(auto& port : ports)`
  static constexpr PortInfo<SerialBusType::BUS_ANY> ports;

  namespace usb
  {
    /// Enumerator range for USB ports
    static constexpr PortInfo<SerialBusType::BUS_USB> ports;
  }
  namespace pci
  {
    /// Enumerator range for PCI ports
    static constexpr PortInfo<SerialBusType::BUS_PCI> ports;
  }
  namespace pnp
  {
    /// Enumerator range for PNP/ISA ports
    static constexpr PortInfo<SerialBusType::BUS_PNP> ports;
  }
  namespace platform
  {
    /// Enumerator range for Platform ports
    static constexpr PortInfo<SerialBusType::BUS_PLATFORM> ports;
  }
}

#endif
