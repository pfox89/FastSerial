// For stream outputs
#include <ostream>
#include <iomanip>

#include "SerialEnumeration.hpp"


namespace Serial {

  const char* to_cstring(BusType type)
  {
    switch (type)
    {
    case BusType::BUS_USB:
      return "USB";
    case BusType::BUS_PCI:
      return "PCI";
    case BusType::BUS_PNP:
      return "PNP";
    case BusType::BUS_PLATFORM:
      return "Platform";
    default:
      return "";
    }
  }

  std::ostream& operator<<(std::ostream& os, BusType type) noexcept
  {
    return os << to_cstring(type);
  }

  std::ostream& operator<<(std::ostream& os, const DeviceInfo& port) noexcept
  {
    static constexpr std::streamsize width = 16;
    os.setf(std::ios::left);
    if (port.id == nullptr) return os << "Invalid device\n";
    os << port.id << '\n';

    if (port.path != nullptr)
    {
      os << std::setw(width) << "  Path:" << port.path << '\n';
    }

    os << std::setw(width) << "  Type:" << port.type << '\n';
    if (port.manufacturer != nullptr)
      os << std::setw(width) << "  Manufacturer:" << port.manufacturer << '\n';
    if (port.name != nullptr)
      os << std::setw(width) << "  Name:" << port.name << '\n';;
    if (port.description != nullptr)
      os << std::setw(width) << "  Description:" << port.description << '\n';

    return os << std::setw(width) << "  VID:" << port.vid << '\n'
      << std::setw(width) << "  PID:" << port.pid << '\n';
  }
}

