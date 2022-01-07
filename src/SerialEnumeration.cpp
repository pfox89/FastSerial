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


static int SerialDevicePathNode_PathNode_print(const SerialDevicePathNode& node, char* buffer, int size) noexcept;

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

  DECLSPEC const char* SerialEnum_LocationType_to_cstring(LocationType type)
  {
    switch (type)
    {
    case LocationType::PCI_ROOT:
      return "PCI";
    case LocationType::PCI_SLOT:
      return "Slot";
    case LocationType::USB_ROOT:
      return "USB";
    case LocationType::USB_PORT:
      return "Port";
    default:
      return "";
    }
  }

  DECLSPEC int SerialDevicePathNode_Path_print(const SerialDeviceInfo& port, char* buffer, int size)
  {
    int count = 0;
    bool next = false;
    for (auto& node : Serial::Path(port))
    {
      if (next) {
        if (count < size)
          buffer[count++] = ',';
        if (count < size)
          buffer[count++] = ' ';
      }
      else
        next = true;
      int pcount = SerialDevicePathNode_PathNode_print(node, &buffer[count], size - count);
      if (pcount < 0) return pcount;
      count += pcount;
    }
    if (count < size)
    {
      buffer[count] = '\0';
    }
    else
    {
      buffer[size - 1] = '\0';
    }
    return count;
  }
  
}

int SerialDevicePathNode_PathNode_print(const SerialDevicePathNode& node, char* buffer, int size) noexcept
{
  return snprintf(buffer, size, "%s %hX", SerialEnum_LocationType_to_cstring(node.type), node.number);
}

DECLSPEC std::ostream& operator<<(std::ostream& os, SerialBusType type) noexcept
{
  return os << SerialBusType_to_cstring(type);
}

DECLSPEC std::ostream& operator<<(std::ostream& os, const SerialDevicePathNode& node) noexcept
{
  char buf[8];
  int count = SerialDevicePathNode_PathNode_print(node, buf, sizeof(buf));
  if (count > 0) return os.write(buf, count);
  else return os;
}

DECLSPEC std::ostream& operator<<(std::ostream& os, const SerialDeviceInfo& port) noexcept
{
  static constexpr std::streamsize width = 16;
  os.setf(std::ios::left);
  if(port.lpath == nullptr) return os << "Invalid device\n";
  os << port.lpath << '\n';

  if (port.path != nullptr)
  {
    int size = 250;
    std::unique_ptr<char[]> path = std::make_unique<char[]>(size);

    int required = SerialDevicePathNode_Path_print(port, &path[0], size);
    if (size < required)
    {
      path = std::make_unique<char[]>(required);
      size = required;
      required = SerialDevicePathNode_Path_print(port, &path[0], size);
    }
    if(required > 0)
      os << std::setw(width) << "  Path:" << path.get() << '\n';
    
   //os << port.path << '\n';
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
