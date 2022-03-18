# include/SerialEnumeration.hpp


## Namespaces

| Name           |
| -------------- |
| **[Serial](namespace_serial.md)**  |
| **[Serial::usb](namespace_serial_1_1usb.md)**  |
| **[Serial::pci](namespace_serial_1_1pci.md)**  |
| **[Serial::pnp](namespace_serial_1_1pnp.md)**  |
| **[Serial::platform](namespace_serial_1_1platform.md)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Serial::DeviceInfo](struct_serial_1_1_device_info.md)** <br>Information about device.  |
| struct | **[Serial::Enum](struct_serial_1_1_enum.md)**  |
| struct | **[Serial::PortIter](struct_serial_1_1_port_iter.md)** <br>Iterator to iterate over ports enumerator.  |
| struct | **[Serial::PortInfo](struct_serial_1_1_port_info.md)** <br>Constexpr range template to allow enumerator to be used as a range.  |




## Source code

```cpp
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
enum class BusType
{
  BUS_UNKNOWN  = 0,
  BUS_USB      = 0x1,
  BUS_PCI      = 0x2,
  BUS_PNP      = 0x4,
  BUS_PLATFORM = 0x8,
  BUS_ANY      = BUS_USB | BUS_PCI | BUS_PNP | BUS_PLATFORM
};

struct DeviceInfo
{
  const char* id;
  const char* path;
  const char* name;
  const char* description;
  const char* manufacturer;
  unsigned short vid;
  unsigned short pid;
  BusType type;
};

struct Enum
{
  Enum()            = default;
  Enum(const Enum& other) = delete;

  Enum(Enum&& other) noexcept
  {
    _impl       = other._impl;
    other._impl = nullptr;
  }

  int getInfoFor(DeviceInfo& info, const void* id) noexcept;

  int begin(unsigned int type_mask=static_cast<unsigned int>(BusType::BUS_ANY)) noexcept;

  int next(DeviceInfo& info) noexcept;

  void clear() noexcept;

  ~Enum() noexcept { clear(); }

private:
  SerialEnumImpl* _impl;
};

const char* to_cstring(BusType type);

inline std::string to_string(BusType type) noexcept { return to_cstring(type); }

std::ostream& operator<<(std::ostream& os, BusType type) noexcept;

std::ostream& operator<<(std::ostream& os, const DeviceInfo& port) noexcept;

struct PortIter
{
  PortIter(unsigned type_mask) noexcept
    : _enum()
    , _status(_enum.begin(type_mask))
    , _info{}
  {
    // Populate info immediately
    ++(*this);
  }

  PortIter(int) noexcept
    : _enum()
    , _status(INT_MAX)
    , _info{}
  {}

  PortIter(const PortIter&) = delete;

  PortIter(PortIter&& other) noexcept
    :  _enum(std::move(other._enum))
     , _status(other._status)
    , _info(other._info)
  {
    other._status = INT_MAX;
  }

  PortIter& operator++() noexcept
  {
    if (_status <= 1) { _status = _enum.next(_info); }

    return *this;
  }

#ifdef __cpp_exceptions
  const DeviceInfo& operator*() const
  {
    if (_status <= 0) throw_error();
    return _info;
  }

  const DeviceInfo* operator->() const
  {
    if (_status <= 0) throw_error();
    return &_info;
  }
#else
  const SerialDeviceInfo& operator*() const noexcept { return _info; }

  const SerialDeviceInfo* operator->() const noexcept { return &_info; }
#endif

  bool operator!=(const PortIter&) const noexcept { return (_status > 0); }

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

template<BusType type>
struct PortInfo
{
  PortIter begin() const noexcept { return PortIter(static_cast<unsigned>(type)); }
  PortIter end() const noexcept { return PortIter(INT32_MAX); }
};

static constexpr PortInfo<BusType::BUS_ANY> ports;

namespace usb
{
  static constexpr PortInfo<BusType::BUS_USB> ports;
}
namespace pci
{
  static constexpr PortInfo<BusType::BUS_PCI> ports;
}
namespace pnp
{
  static constexpr PortInfo<BusType::BUS_PNP> ports;
}
namespace platform
{
  static constexpr PortInfo<BusType::BUS_PLATFORM> ports;
}

} // End namespace Serial
```


-------------------------------

Updated on 2022-03-18 at 13:06:36 -0400
