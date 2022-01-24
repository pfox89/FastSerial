#pragma once

#include <iosfwd>
#include <string>
#include <system_error>

#ifdef __cpp_exceptions
#include <stdexcept>
#endif

namespace
{
struct SerialEnumImpl;
}

namespace Serial
{
/// Possible types of bus this device can be connected to
enum class BusType
{
  BUS_UNKNOWN  = 0,
  BUS_USB      = 0x1,
  BUS_PCI      = 0x2,
  BUS_PNP      = 0x4,
  BUS_PLATFORM = 0x8,
  BUS_ANY      = BUS_USB | BUS_PCI | BUS_PNP | BUS_PLATFORM
};

/// Information about device
struct DeviceInfo
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
  BusType type;
};

/// Serial Enumerator class, which manages resource associated with serial device enumeration
/// In general, begin() must be called to initate enumeration, and next() called to get subsequent devices until it returns <= 0
struct Enum
{
  Enum()            = default;
  Enum(const Enum& other) = delete;

  /// Move constructor allows moving pointer
  Enum(Enum&& other) noexcept
  {
    _impl       = other._impl;
    other._impl = nullptr;
  }

  /// \brief Get info for given device ID
  /// \param[out] info Reference of struct to store info of device
  /// \param[in]  id   ID of device. This will be a utf-8 encoded syspath on Linux, or a Windows device ID encoded as LPWSTR
  /// \return == 0 No device found
  ///          > 0 Device info retrieved successfully
  ///          < 0 Error occurred, std::error_code(-return, system_category()) for details.
  int getInfoFor(DeviceInfo& info, const void* id) noexcept;

  /// \brief Start enumerating devices of given types
  /// \param type_mask Bitwise combination of any BusType values defining what kind of devices to include
  /// \return == 0 Successful
  ///          < 0 Error occurred, std::error_code(-return, system_category()) for details.
  int begin(unsigned int type_mask=static_cast<unsigned int>(BusType::BUS_ANY)) noexcept;

  /// \brief Get info for next device in enumeration
  /// \param[out] info Reference of struct to store info of device
  /// \return == 0 No more devices found
  ///          > 0 Device info retrieved successfully
  ///          < 0 Error occurred, std::error_code(-return, system_category()) for details.
  /// \note All strings are bound to the current enumeration state, so strings in DeviceInfo 
  ///       should be copied before calling any other member functions if they will be needed later.
  int next(DeviceInfo& info) noexcept;

  /// Free all resources associated with enumeration. This will invalidate DeviceInfo strings.
  void clear() noexcept;

  ~Enum() noexcept { clear(); }

private:
  SerialEnumImpl* _impl;
};

const char* to_cstring(BusType type);

inline std::string to_string(BusType type) noexcept { return to_cstring(type); }

std::ostream& operator<<(std::ostream& os, BusType type) noexcept;

/// Stream insertion operator to pretty-print serial device information
std::ostream& operator<<(std::ostream& os, const DeviceInfo& port) noexcept;

/// Iterator to iterate over ports enumerator
struct PortIter
{
  /// Create iterator over ports using type mask
  /// \param type_mask Bitwise combination of any BusType values defining what kind of devices to include
  PortIter(unsigned type_mask) noexcept
    : _enum()
    , _status(_enum.begin(type_mask))
    , _info{}
  {
    // Populate info immediately
    ++(*this);
  }

  /// Passing an int constructs an dummy "end" iterator by setting status to invalid value
  PortIter(int) noexcept
    : _enum()
    , _status(INT_MAX)
    , _info{}
  {}

  /// Don't allow copying iterator, ownership must be retained
  PortIter(const PortIter&) = delete;

  /// Move iterator takes ownership
  PortIter(PortIter&& other) noexcept
    :  _enum(std::move(other._enum))
     , _status(other._status)
    , _info(other._info)
  {
    other._status = INT_MAX;
  }

  /// Incrementing iterator gets next device
  PortIter& operator++() noexcept
  {
    if (_status <= 1) { _status = _enum.next(_info); }

    return *this;
  }

#ifdef __cpp_exceptions
  /// Get reference to device info for current enumerated device
  const DeviceInfo& operator*() const
  {
    if (_status <= 0) throw_error();
    return _info;
  }

  /// Get pointer to device info for current enumerated device
  const DeviceInfo* operator->() const
  {
    if (_status <= 0) throw_error();
    return &_info;
  }
#else
  /// Get reference to device info for current enumerated device (exceptions disabled, no check for validity)
  const SerialDeviceInfo& operator*() const noexcept { return _info; }

  /// Get pointer to device info for current enumerated device (exceptions disabled, no check for validity)
  const SerialDeviceInfo* operator->() const noexcept { return &_info; }
#endif

  /// Comparison operator does not inspect other iterator, only current status to determine the enumeration has reached the end 
  bool operator!=(const PortIter&) const noexcept { return (_status > 0); }

  /// Get error that occurred while enumerating, if any 
  std::error_code error() const noexcept { return std::error_code(_status < 0 ? -_status : 0, std::system_category()); }

private:
  Enum       _enum;
  int        _status;
  DeviceInfo _info;

#ifdef __cpp_exceptions
  void throw_error() const
  {
    if (_status > 0) throw std::system_error(error());
    else if (_status < 0)
      throw std::out_of_range("Serial::PortIter");
  }
#endif
};

/// Constexpr range template to allow enumerator to be used as a range
template<BusType type>
struct PortInfo
{
  /// Get iterator to begin enumeration
  PortIter begin() const noexcept { return PortIter(static_cast<unsigned>(type)); }
  /// Get dummy (invalid) iterator for end of enumeration
  PortIter end() const noexcept { return PortIter(INT32_MAX); }
};

/// \brief Enuemrator object for all ports
/// Use begin() and end() functions to get iterators to port range,
/// or use in range-based for loop, e.g. `for(auto& port : ports)`
static constexpr PortInfo<BusType::BUS_ANY> ports;

namespace usb
{
  /// Enumerator range for USB ports
  static constexpr PortInfo<BusType::BUS_USB> ports;
}
namespace pci
{
  /// Enumerator range for PCI ports
  static constexpr PortInfo<BusType::BUS_PCI> ports;
}
namespace pnp
{
  /// Enumerator range for PNP/ISA ports
  static constexpr PortInfo<BusType::BUS_PNP> ports;
}
namespace platform
{
  /// Enumerator range for Platform ports
  static constexpr PortInfo<BusType::BUS_PLATFORM> ports;
}

} // End namespace Serial
