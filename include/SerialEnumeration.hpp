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

struct Enum
{
  Enum() noexcept;
  int getInfoFor(DeviceInfo& info, const void* id) noexcept;
  int begin(unsigned int type_mask) noexcept;
  int next(DeviceInfo& info) noexcept;
  ~Enum() noexcept;

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
  // Create iterator over ports using type mask
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
    : _status(INT_MAX)
    , _info{}
  {}

  /// Don't allow copying iterator, ownership must be retained
  PortIter(const PortIter&) = delete;

  /// Move iterator takes ownership
  PortIter(PortIter&& other) noexcept
    : _status(other._status)
    , _info(other._info)
  {
    other._status = INT_MAX;
  }

  /// Incrementing iterator gets next device
  PortIter& operator++() noexcept
  {
    if (_status == 0) { _status = _enum.next(_info); }

    return *this;
  }

#ifdef __cpp_exceptions
  const DeviceInfo& operator*() const
  {
    if (_status != 0) throw_error();
    return _info;
  }

  const DeviceInfo* operator->() const
  {
    if (_status != 0) throw_error();
    return &_info;
  }
#else
  const SerialDeviceInfo& operator*() const noexcept { return _info; }

  const SerialDeviceInfo* operator->() const noexcept { return &_info; }
#endif

  bool operator!=(const PortIter&) const noexcept { return (_status == 0); }

  std::error_code error() const noexcept { return std::error_code(_status < 0 ? 0 : _status, std::system_category()); }

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
  PortIter begin() const noexcept { return PortIter(static_cast<unsigned>(type)); }
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
}
