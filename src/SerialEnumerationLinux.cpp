#include <memory>
#include <cerrno>
#include <system_error>
#include <cstring>

#include "SerialEnumeration.h"

#include <libudev.h>

template <typename D, D fn>
using deleter_from_fn = std::integral_constant<D, fn>;

template <typename T, T *fn(T*)>
using udev_obj_ptr = std::unique_ptr<T, deleter_from_fn<decltype(fn), fn>>;

using udev_ptr = udev_obj_ptr<udev, udev_unref>;
using udev_device_ptr = udev_obj_ptr<udev_device, udev_device_unref>;
using udev_enumerate_ptr = udev_obj_ptr<udev_enumerate, udev_enumerate_unref>;

struct UDevEnumeration
{
  UDevEnumeration() noexcept = default;
  UDevEnumeration(const UDevEnumeration& other) = delete;

  int init(unsigned int type) noexcept
  {
    _typeMask = type;

    _udev = udev_ptr(udev_new());
    if(_udev == nullptr) return errno;
    _enumerate = udev_enumerate_ptr(udev_enumerate_new(_udev.get()));
    if(_enumerate == nullptr) return errno;
    if( udev_enumerate_add_match_subsystem(_enumerate.get(), "tty") < 0)
      return errno;
    int stat;
    switch(static_cast<SerialBusType>(type))
    {
      case SerialBusType::BUS_USB:
        stat = udev_enumerate_add_match_property(_enumerate.get(), "ID_BUS", "usb");
        break;
      case SerialBusType::BUS_PCI:
        stat = udev_enumerate_add_match_property(_enumerate.get(), "ID_BUS", "pci");
        break;
      default:
        // Other busses do not populate ID_BUS, so we have to filter later
        stat = 0;
        break;
    }
    if(stat < 0) return errno;
    if( udev_enumerate_scan_devices(_enumerate.get()) < 0)
      return errno;
    
    return 0;
  }

  int next(SerialDeviceInfo& info) noexcept
  {
    static constexpr char virtual_path[] = "/sys/devices/virtual";
    do {
      do {
        if(_devices == nullptr)
          _devices = udev_enumerate_get_list_entry(_enumerate.get());
        else
        {
          _devices = udev_list_entry_get_next(_devices);
        }

        if(_devices == nullptr) return -1;
        info.lpath = udev_list_entry_get_name(_devices);
        if(info.lpath == nullptr) return errno;
        // Skip virtual ttys
      } while(strncmp(info.lpath, virtual_path, sizeof(virtual_path)-1) == 0);

      _dev = udev_device_ptr(udev_device_new_from_syspath(_udev.get(), info.lpath));
      if(_dev == nullptr) return errno;

      info.name = udev_device_get_sysname(_dev.get());
      const char* bus = udev_device_get_property_value(_dev.get(), "ID_BUS");
      
      const char* vid_str = udev_device_get_property_value(_dev.get(), "ID_VENDOR_ID");
      if(vid_str != nullptr) info.vid = strtoul(vid_str, nullptr, 16);
      else info.vid = 0;
      const char* pid_str = udev_device_get_property_value(_dev.get(), "ID_MODEL_ID");
      if(pid_str != nullptr) info.pid = strtoul(pid_str, nullptr, 16);
      else info.pid = 0;

      info.manufacturer = udev_device_get_property_value(_dev.get(), "ID_VENDOR");
      info.path = nullptr;

      if(bus == nullptr)
      {
        udev_device* parent;
        if((parent = udev_device_get_parent_with_subsystem_devtype(_dev.get(), "pnp", nullptr)) != nullptr)
        {
          // Skip this device if we did not request PNP or any devices 
          if(0 == (_typeMask & static_cast<unsigned>(SerialBusType::BUS_PNP)))
            continue;

          info.description = udev_device_get_sysattr_value(parent, "id");
          // Legacy PNP devices don't report friendly names, so add friendly name for known serial PNP devices
          if(strncmp(info.description, "PNP050", 6) == 0)
          {
            if(info.description[6] == '1')
              info.description = "16550A-compatible PNP Serial Device";
            else
              info.description = "Standard PNP Serial Device";
          }

          info.path = get_path(parent);

          info.type = SerialBusType::BUS_PNP;
        }
        else if((parent = udev_device_get_parent_with_subsystem_devtype(_dev.get(), "platform", nullptr)) != nullptr)
        {
          // Skip this device if we did not request PNP or any devices
          if(0 == (_typeMask & static_cast<unsigned>(SerialBusType::BUS_PLATFORM)))
            continue;

          info.description = udev_device_get_driver(parent);
          // The only info we can get for platform devices is the driver, so add friendly name for most common PC hardware
          if(strcmp(info.description, "serial8250") == 0)
            info.description = "8250/16550 Series Platform Serial Device";

          info.type = SerialBusType::BUS_PLATFORM;
        }
        else
        {
          if( _typeMask != static_cast<unsigned>(SerialBusType::BUS_ANY)) continue;

          info.description = nullptr;
          info.type = SerialBusType::BUS_UNKNOWN;
        }
        info.manufacturer = nullptr;
        break;
      }
      else if(strncmp(bus, "usb", 3) == 0)
      {
        if(0 == (_typeMask & static_cast<unsigned>(SerialBusType::BUS_USB)))
        {
          // If we didn't ask for a USB device or any, skip this device
          continue;
        }

        info.description = udev_device_get_property_value(_dev.get(), "ID_MODEL_FROM_DATABASE");
        if(info.description == nullptr 
          || (strcasestr(info.description, "controller") != nullptr) 
          || (strcasestr(info.description, "hub") != nullptr) // Workaround for funky udev bug where it shows the parent device model
          )
        {
          info.description = udev_device_get_property_value(_dev.get(), "ID_MODEL");
        }

        udev_device* parent = udev_device_get_parent_with_subsystem_devtype(_dev.get(), "usb", nullptr);
        
        info.path = get_path(parent);

        info.type = SerialBusType::BUS_USB;
        break;
      }
      else if(strncmp(bus, "pci", 3) == 0)
      {
        if(0 == (_typeMask & static_cast<unsigned>(SerialBusType::BUS_PCI)))
        {
          continue;
        }
       
        // domain:bus:slot.function
        udev_device* parent = udev_device_get_parent_with_subsystem_devtype(_dev.get(), "pci", nullptr);
        
        info.path = get_path(parent);
        
        info.description = udev_device_get_property_value(_dev.get(), "ID_MODEL_FROM_DATABASE");
        info.manufacturer = udev_device_get_property_value(_dev.get(), "ID_VENDOR_FROM_DATABASE");
        info.type = SerialBusType::BUS_PCI;
        break;
      }
      else
      {
        if( _typeMask != static_cast<unsigned>(SerialBusType::BUS_ANY)) continue;
        info.description = nullptr;
        info.type = SerialBusType::BUS_UNKNOWN;
        break;
      }
    } while(1);
    
    return 0;
  }

private:

  const char* get_path(udev_device* parent) noexcept
  {
    const char* ppath = udev_device_get_devpath(parent);
    SerialBusType type = SerialBusType::BUS_UNKNOWN;

    if(ppath != nullptr)
    {
      const char* phypath = strchr(++ppath, '/');

      if (phypath == nullptr) return nullptr;

      size_t strsize = strlen(phypath);
      if (_pathBufferSize < strsize)
      {
        _pathBufferSize = std::max((_pathBufferSize * 3) / 2, strsize));
        _pathBuffer = std::make_unique<char[]>(_pathBufferSize);
        if (!_pathBuffer)
        {
          _pathBufferSize = 0;
          return nullptr;
        }
      }
      char* dest = _pathBuffer.get();

      const char* cpath_next;

      while(phypath != nullptr)
      {

        if (strncmp(phypath, "pci", 3) == 0)
        {
          phypath += 3;
          // Skip to the bus number, we don't record PCI domain
          phypath = strchr(phypath, ':');
          if (phypath != nullptr)
          {
            memcpy(dest, "PCI ", 5);
            dest += 5;
            ++phypath;
            if (std::isxdigit(*phypath)) *dest++ = *phypath++;
            if (std::isxdigit(*phypath)) *dest++ = *phypath++;

            phypath = strchr(phypath, '/');
            if (phypath == nullptr) break;
              
            ++phypath;

            // Format should be XXXX:XX:XX with domain, bus, slot in the fields
            if (phypath[4] == ':' && phypath[7] == ':' && std::isxdigit(phypath[8]) && std::isxdigit(phypath[9]))
            {
              memcpy(dest, ", Device ", 9);
              *dest++ = phypath[8];
              *dest++ = phypath[9];

              phypath += 10;
              phypath = strchr(phypath, '/');
            }
          }
        }
        else if (strncmp(phypath, "usb", 3) == 0)
        {
          memcpy(dest, "USB ", 4);
          dest += 4;

          phypath += 3;
          // Look for interface to get node that contains full path of root_hub-port-port-port-...:config.interface
          cpath_next = strchr(phypath, ':');
          if (cpath_next == nullptr) break;
          // Search backwards for last '/' before ':' to find full path
          while (cpath_next > phypath && *cpath_next-- != '/')
          {}
          // First digit is root hub number
          phypath = ++cpath_next;
          if (false == std::isxdigit(*phypath)) break;

          *dest++ = *phypath++;
          if (std::isxdigit(*phypath)) *dest++ = *phypath++;

          // Subsequent digits, delimited by '-' are port numbers
          while(phypath < cpath_next && *phypath++ == '-' && std::isxdigit(phypath[0]))
          {
            memcpy(dest, ", Port ", 7);
            dest += 7;
            *dest++ = *phypath++;
            if (std::isxdigit(*phypath))
            {
              *dest++ = *phypath++;
            }
          }

          break;
        }
        if (phypath != nullptr) ++phypath;
      }
      if (dest != nullptr) *dest = '\0';
    }

    return _pathBuffer.get();
    else return nullptr;
  }

  unsigned                _typeMask;
  udev_ptr                _udev;
  udev_device_ptr         _dev;
  udev_enumerate_ptr      _enumerate;
  udev_list_entry*        _devices;
  std::unique_ptr<char[]> _pathBuffer;
  size_t                  _pathBufferSize;
};

thread_local std::unique_ptr<UDevEnumeration> enum_ptr;

extern "C"
{
int SerialEnum_StartEnumeration(unsigned int typeMask)
{
  enum_ptr = std::make_unique<UDevEnumeration>();
  if (!enum_ptr)
    return static_cast<int>(std::errc::not_enough_memory);
  return enum_ptr->init(typeMask);
}

int SerialEnum_Next(SerialDeviceInfo *info)
{
  if (info == nullptr)
    return static_cast<int>(std::errc::invalid_argument);
  if (!enum_ptr)
    return static_cast<int>(std::errc::not_connected);
  return enum_ptr->next(*info);
}

  void SerialEnum_Finish()
  {
    enum_ptr.reset();
  }
}
