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

/// Information about device
struct SerialDeviceInfo
{
  /// Logical device path (e.g. interface)
  const char* id;
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
  /// \return Status/error code of system_category type
  /// \remarks Always call SerialEnum_Finish after this function to free resources allocated
  DECLSPEC int SerialEnum_StartEnumeration(unsigned int typeMask);

  /// Gets info for next device
  /// \param[out] info Pointer to info struct to receive device info
  /// \return Status/error code of system_category type
  /// \remark Strings remain valid only until another SerialEnum function is called, so they should be copied
  DECLSPEC int SerialEnum_Next(SerialDeviceInfo* info);

  /// \brief Gets info for specified device
  /// \param[out] info Pointer to info struct to receive device info
  /// \param[in] id Pointer to device id string.
  ///               On Windows, this should be a null-terminated WCHAR* string with a device interface path.
  ///               On Linux it should be null-terminated char* sysfs path to the device
  /// \return Status/error code of system_category type
  /// \remark Should be called after after SerialEnum_StartEnumeration() and before SerialEnum_Finish(). All strings
  DECLSPEC int SerialEnum_GetDeviceInfo(SerialDeviceInfo* info, const void* id);

  /// Finish enumeration, freeing all associated resources
  DECLSPEC void SerialEnum_Finish();

  /// Get String describing SerialBusType
  DECLSPEC const char* SerialBusType_to_cstring(SerialBusType type);


#ifdef __cplusplus 
}

/// Stream insertion operator to print SerialBusType
DECLSPEC std::ostream& operator<<(std::ostream& os, SerialBusType type) noexcept;

/// Stream insertion operator to pretty-print serial device information
DECLSPEC std::ostream& operator<<(std::ostream& os, const SerialDeviceInfo& port) noexcept;

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
