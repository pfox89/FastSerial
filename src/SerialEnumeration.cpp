// For stream outputs
#include <ostream>
#include <iomanip>

#include "SerialEnumeration.h"

#if defined(_WIN32)
#  if defined(EXPORTING_SERIALENUM)
#    define DECLSPEC __declspec(dllexport)
#  else defined(SERIAL_STATIC)
#    define DECLSPEC
#  endif
#else // non windows
#  define DECLSPEC [[gnu::visibility("default")]]
#endif

extern "C"
{
  DECLSPEC const char* SerialBusType_to_cstring(SerialBusType type)
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

DECLSPEC std::ostream& operator<<(std::ostream& os, SerialBusType type) noexcept
{
  return os << SerialBusType_to_cstring(type);
}

DECLSPEC std::ostream& operator<<(std::ostream& os, const SerialDeviceInfo& port) noexcept
{
  static constexpr std::streamsize width = 16;
  os.setf(std::ios::left);
  if(port.id == nullptr) return os << "Invalid device\n";
  os << port.id << '\n';

  if (port.path != nullptr)
  {
    os << std::setw(width) << "  Path:" << port.path << '\n';
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
