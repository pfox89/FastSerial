// For stream outputs
#include <ostream>
#include <iomanip>

#include "SerialEnumeration.h"

#ifdef _WIN32
#define EXPORTING __declspec(dllexport)
#else
#define EXPORTING [[gnu::visibility("default")]]
#endif

extern "C"
{
  EXPORTING const char* to_cstring(SerialBusType type)
{
  switch (type)
  {
  case SerialBusType::BUS_USB:
    return "USB";
  case SerialBusType::BUS_PCI:
    return "PCI";
  case SerialBusType::BUS_PNP:
    return "PNP";
  case SerialBusType::BUS_PLATFORM:
    return "Platform";
  default:
    return "";
  }
}
}

EXPORTING std::ostream& operator<<(std::ostream& os, SerialBusType type) noexcept
{
  return os << to_cstring(type);
}

std::ostream& operator<<(std::ostream& os, const SerialDevicePathNode& node) noexcept
{
  switch (node.type)
  {
  case LocationType::PCI_ROOT:
    os << "PCI";
    break;
  case LocationType::PCI_SLOT:
    os << "Slot";
    break;
  case LocationType::USB_ROOT:
    os << "USB";
    break;
  case LocationType::USB_PORT:
    os << "Port";
    break;
  default:
    break;
  }
  return os << ' ' << std::setbase(std::ios::hex) << node.number;
}
EXPORTING std::ostream& operator<<(std::ostream& os, const SerialDeviceInfo& port) noexcept
{
  static constexpr std::streamsize width = 16;
  os.setf(std::ios::left);
  if(port.lpath == nullptr) return os << "Invalid device\n";
  os << port.lpath << '\n';
  if (port.path != nullptr)
  {
    os << std::setw(width) << "  Path:";

    bool next = false;
    for (auto& node : Serial::Path(port))
    {
      if (next) {
        os << ", ";
      }
      else
        next = true;

      os << node;
    }

    os << '\n';
  }
  os << std::setw(width) << "  Type:" << port.type << '\n';
  if(port.manufacturer != nullptr)
    os << std::setw(width) << "  Manufacturer:" << port.manufacturer << '\n';
  if(port.name != nullptr)
    os << std::setw(width) << "  Name:" << port.name << '\n';;
  if(port.description != nullptr)
    os << std::setw(width) << "  Description:" << port.description << '\n';

  return os << std::setw(width) << "  VID:" << port.vid << '\n'
     << std::setw(width) << "  PID:" << port.pid << '\n';
}
