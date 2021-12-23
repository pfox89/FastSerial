#include <memory>
#include <cerrno>
#include <system_error>
#include <cstring>


#include "SerialEnumeration.h"

#include <libudev.h>

struct UDevEnumeration
{

  UDevEnumeration() noexcept = default;
  UDevEnumeration(const UDevEnumeration& other) = delete;

  int init()
  {
    _udev = udev_new();
    if(_udev == nullptr) return errno;
    _enumerate = udev_enumerate_new(_udev);
    if(_enumerate == nullptr) return errno;
    if( udev_enumerate_add_match_subsystem(_enumerate, "tty") < 0)
      return errno;
    // Exclude virtual tty devices
    //if( udev_enumerate_add_match_sysname(_enumerate, "tty[A-z][0-9]*") < 0)
    //  return errno;
    if( udev_enumerate_scan_devices(_enumerate) < 0)
      return errno;

    
    return 0;
  }

  int next(SerialDeviceInfo& info) noexcept
  {
    static constexpr char virtual_path[] = "/sys/devices/virtual";
    do {
      if(_devices == nullptr)
        _devices = udev_enumerate_get_list_entry(_enumerate);
      else
      {
        _devices = udev_list_entry_get_next(_devices);
      }

      if(_devices == nullptr) return -1;
      info.lpath = udev_list_entry_get_name(_devices);
      if(info.lpath == nullptr) return errno;
    } while(strncmp(info.lpath, virtual_path, sizeof(virtual_path)-1) == 0);

    if(_dev != nullptr)
      udev_device_unref(_dev);

    _dev = udev_device_new_from_syspath(_udev, info.lpath);
    if(_dev == nullptr) return errno;

    const char* bus = udev_device_get_property_value(_dev, "ID_BUS");
    if(bus == nullptr)
    {
      udev_device* parent;
      if((parent = udev_device_get_parent_with_subsystem_devtype(_dev, "pnp", nullptr)) != nullptr)
      {
        info.description = udev_device_get_sysattr_value(parent, "id");

        if(strncmp(info.description, "PNP050", 6) == 0)
        {
          if(info.description[6] == '1')
            info.description = "16550A-compatible PNP Serial Device";
          else
            info.description = "Standard PNP Serial Device";
        }

        info.type = SerialBusType::BUS_ISA;
      }
      else if((parent = udev_device_get_parent_with_subsystem_devtype(_dev, "platform", nullptr)) != nullptr)
      {
        info.description = udev_device_get_driver(parent);
        if(strcmp(info.description, "serial8250") == 0)
          info.description = "8250-type platform serial device";
        info.type = SerialBusType::BUS_PLATFORM;
      }
      else
      {
        info.description = nullptr;
        info.type = SerialBusType::BUS_UNKNOWN;
      }
      info.manufacturer = nullptr;
      info.vid = 0;
      info.pid = 0;
    }
    else if(strncmp(bus, "usb", 3) == 0)
    {
      info.description = udev_device_get_property_value(_dev, "ID_MODEL_FROM_DATABASE");
      if(info.description == nullptr 
         || (strcasestr(info.description, "controller") != nullptr) // Workaround for funky udev bug where it uses the parent device model
         )
      {
         info.description = udev_device_get_property_value(_dev, "ID_MODEL");
      }
      const char* vid_str = udev_device_get_property_value(_dev, "ID_VENDOR_ID");
      if(vid_str != nullptr) info.vid = strtoul(vid_str, nullptr, 16);
      const char* pid_str = udev_device_get_property_value(_dev, "ID_MODEL_ID");
      if(pid_str != nullptr) info.pid = strtoul(pid_str, nullptr, 16);
      info.manufacturer = udev_device_get_property_value(_dev, "ID_VENDOR");
      info.type = SerialBusType::BUS_USB;
    }
    else if(strncmp(bus, "pci", 3) == 0)
    {
      info.description = udev_device_get_property_value(_dev, "ID_MODEL_FROM_DATABASE");
      info.manufacturer = udev_device_get_property_value(_dev, "ID_VENDOR_FROM_DATABASE");
      info.type = SerialBusType::BUS_PCI;
    }

    return 0;
  }

  ~UDevEnumeration() noexcept
  {
    if(_dev != nullptr)
      udev_device_unref(_dev);
    udev_enumerate_unref(_enumerate);
    udev_unref(_udev);
    _dev = nullptr;
    _udev = nullptr;
    _enumerate = nullptr;
    _devices = nullptr;
  }

private:
  udev_device* _dev;
  udev* _udev;
  udev_enumerate* _enumerate;
  udev_list_entry* _devices;
};

thread_local std::unique_ptr<UDevEnumeration> enum_ptr;

extern "C"
{
  int SerialEnum_StartEnumeration()
  {
    enum_ptr = std::make_unique<UDevEnumeration>();
    if(!enum_ptr) return static_cast<int>(std::errc::not_enough_memory);
    return  enum_ptr->init();
  }

  int SerialEnum_Next(SerialDeviceInfo* info)
  {
    if(info == nullptr) return static_cast<int>(std::errc::invalid_argument);
    if(!enum_ptr) return static_cast<int>(std::errc::not_connected);
    return enum_ptr->next(*info);
  }

  void SerialEnum_Finish()
  {
    enum_ptr.reset();
  }
}
