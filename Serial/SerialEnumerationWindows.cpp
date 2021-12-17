
#define API __declspec(dllexport)
#include "SerialEnumeration.h"
#undef API

#include <ostream>
#include <comdef.h>
#include <SetupAPI.h>
#include <Devguid.h>
#include <initguid.h>

#include <Cfgmgr32.h>

struct DevInfoSet
{
  HDEVINFO _info;

  DevInfoSet() : _info(INVALID_HANDLE_VALUE) {}
  DevInfoSet(const DevInfoSet&) = delete;
  DevInfoSet& operator=(const DevInfoSet&) = delete;

  DevInfoSet& operator=(HDEVINFO other)
  {
    destroy();
    _info = other;
    return *this;
  }

  DevInfoSet(HDEVINFO info) : _info(info) {}

  void destroy()
  {
    if (_info != INVALID_HANDLE_VALUE)
    {
      SetupDiDestroyDeviceInfoList(_info);
      _info = INVALID_HANDLE_VALUE;
    }
  }

  ~DevInfoSet()
  {
    destroy();
  }

  operator const HDEVINFO() const
  {
    return _info;
  }
};


struct EnumData
{
private:
  DevInfoSet      hDevInfoSet;
  DevInfoSet      hTempInfoSet;
  DWORD           nIndex;
  SP_DEVINFO_DATA devInfo;
  SerialAPIString path;
  SerialAPIString description;
  SerialAPIString name;
  SerialAPIString manufacturer;
  SerialDeviceInfo      info;
  HANDLE          hHeap;

  bool resize(DWORD size, SerialAPIString& string) noexcept
  {
    if (size > string.capacity)
    {
      if (size > UINT16_MAX) size = UINT16_MAX;

      void* newbuf;
      if (string.data == nullptr) newbuf = HeapAlloc(hHeap, 0, size);
      else newbuf = HeapReAlloc(hHeap, 0, string.data, size);

      if (newbuf == nullptr) return false;
      string.data = reinterpret_cast<char*>(newbuf);
      string.capacity = static_cast<uint16_t>(size);
      string.length = static_cast<uint16_t>(size - 1);
    }
    else
    {
      string.length = static_cast<uint16_t>(size - 1);
    }
    return true;
  }

  void reset(SerialAPIString& str) noexcept
  {
    if (str.data != nullptr)
    {
      HeapFree(hHeap, 0, str.data);
      str.data = nullptr;
      str.length = 0;
    }
  }

  CONFIGRET get_device_id(SerialAPIString& str, DWORD deviceInst)
  {
    DWORD dwSize = 0;
    CONFIGRET ret = CM_Get_Device_ID_Size(&dwSize, deviceInst, 0);
    if (ret == CR_SUCCESS)
    {
      resize(++dwSize, str);
      ret = CM_Get_Device_ID(deviceInst, str.data, dwSize, 0);
    }
    if (ret != CR_SUCCESS)
    {
      resize(0, str);
    }
    return ret;
  }

  const SerialAPIString get_reg_property(DevInfoSet& set, SP_DEVINFO_DATA& device, DWORD propertyId, SerialAPIString& str)
  {
    DWORD dwSize;
    if (SetupDiGetDeviceRegistryPropertyA(set, &device, propertyId, nullptr, reinterpret_cast<PBYTE>(str.data), str.capacity, &dwSize))
    {
      resize(dwSize, str);
    }
    else
    {
      DWORD err = GetLastError();
      if (err == ERROR_INSUFFICIENT_BUFFER)
      {
        resize(dwSize, str);
        if (!SetupDiGetDeviceRegistryPropertyA(set, &device, propertyId, nullptr, reinterpret_cast<PBYTE>(str.data), str.capacity, &dwSize))
        {
          resize(dwSize, str);
        }
      }
    }
    return str;
  }

  void get_custom_device_property(DevInfoSet& set, SP_DEVINFO_DATA& device, const char* name, SerialAPIString& str)
  {
    DWORD dwSize;
    if (SetupDiGetCustomDevicePropertyA(set, &device, name, 0, nullptr, reinterpret_cast<PBYTE>(str.data), str.capacity, &dwSize))
    {
      resize(dwSize, str);
    }
    else
    {
      DWORD err = GetLastError();
      if (err == ERROR_INSUFFICIENT_BUFFER)
      {
        resize(dwSize, str);
        if (!SetupDiGetCustomDevicePropertyA(set, &device, name, 0, nullptr, reinterpret_cast<PBYTE>(str.data), str.capacity, &dwSize))
        {
          resize(0, str);
        }
      }
    }
  }

public:
  EnumData() noexcept
    : hDevInfoSet(SetupDiGetClassDevsExA(&GUID_DEVINTERFACE_COMPORT, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE, nullptr, nullptr, nullptr))
    , hTempInfoSet(SetupDiCreateDeviceInfoList(nullptr, nullptr))
    , devInfo{ sizeof(SP_DEVINFO_DATA), 0, 0, 0}
    , nIndex(0)
    , path{}
    , description{}
    , name{}
    , manufacturer{}
    , info{}
    , hHeap(GetProcessHeap())
  {}

  ~EnumData() noexcept
  {
    reset(path);
    reset(description);
    reset(name);
    reset(manufacturer);
  }

  int32_t getError() const noexcept
  {
    if (hDevInfoSet == INVALID_HANDLE_VALUE)
    {
      return GetLastError();
    }
    else
    {
      return S_OK;
    }
  }

  int32_t next() noexcept
  {
    if (!SetupDiEnumDeviceInfo(hDevInfoSet, nIndex++, &devInfo))
    {
      return GetLastError();
    }
    else return S_OK;
  }


  void get_path(DEVINST device, const char* rootstr, size_t rootstr_size, const char* hubstr, size_t hubstr_size, SerialAPIString& str)
  {
    if (get_device_id(str, device) == CR_SUCCESS)
    {
      SP_DEVINFO_DATA tempInfo = { sizeof(SP_DEVINFO_DATA) };

      if (SetupDiOpenDeviceInfoA(hTempInfoSet, str.data, nullptr, 0, &tempInfo))
      {
        SerialAPIString temp{};
        get_reg_property(hTempInfoSet, tempInfo, SPDRP_LOCATION_PATHS, temp);
        if (temp.data != nullptr)
        {
          const char* pos = strstr(temp.data, rootstr);

          if (pos != nullptr)
          {
            const char* posend = strchr(pos, '\0');

            // Reserve plenty of space for this string
            resize(static_cast<uint16_t>(posend - pos), str);
            uint16_t path_size = 0;

            pos += rootstr_size;
            while (std::isdigit(*pos))
            {
              str.data[path_size++] = *pos++;
            }
            while (pos != nullptr && *pos++ == ')' && *pos++ == '#')
            {
              pos = strstr(pos, hubstr);
              if (pos != nullptr)
              {
                str.data[path_size++] = '/';

                pos += hubstr_size;
                while (std::isdigit(*pos))
                {
                  str.data[path_size++] = *pos++;
                }
              }
              // Ensure string is null-terminated and resize
              str.data[path_size++] = '\0';
              resize(path_size, str);
            }
          }
        }
      }
    }
  }


  const SerialDeviceInfo* getDeviceInfo() noexcept
  {
    DWORD dwSize = 0;
    info.vid = 0;
    info.pid = 0;
    info.type = SerialBusType::BUS_UNKNOWN;

    if (get_device_id(path, devInfo.DevInst) == CR_SUCCESS)
    {
      DEVINST parent;

      // If this isn't on a PCI or USB root, it may be a virtual device referencing a base physical device, so find its parent id
      if (strncmp(path.data, "USB", 3) != 0 && strncmp(path.data, "PCI", 3) != 0)
      {
        if (CM_Get_Parent(&parent, devInfo.DevInst, 0) == CR_SUCCESS)
        {
          get_device_id(path, parent);
        }
      }
      else
      {
        parent = devInfo.DevInst;
      }

      if (path.length > 2)
      {
        if (strncmp(path.data, "USB", 3) == 0)
        {
          info.type = SerialBusType::BUS_USB;

          {
            const char* pos = strstr(&path.data[3], "VID_");
            if (pos != nullptr)
            {
              char* next;
              info.vid = static_cast<uint16_t>(strtoul(pos + 4, &next, 10));
              pos = next;

              pos = strstr(pos, "PID_");
              if (pos != nullptr)
              {
                info.pid = static_cast<uint16_t>(strtoul(pos + 4, &next, 10));
                pos = next;
              }
            }
          }

          get_path(parent, "USBROOT", sizeof("USBROOT"), "USB", sizeof("USB"), path);

        }
        else if (strncmp(path.data, "PCI", 3) == 0)
        {
          info.type = SerialBusType::BUS_PCI;
          const char* pos = strstr(&path.data[3], "VEN_");
          if (pos != nullptr)
          {
            char* next;
            info.vid = static_cast<uint16_t>(strtoul(pos + 4, &next, 10));
            pos = next;

            pos = strstr(pos, "DEV_");
            if (pos != nullptr)
            {
              info.pid = static_cast<uint16_t>(strtoul(pos + 4, &next, 16));
              pos = next;
            }
          }

          get_path(parent, "PCIROOT", sizeof("PCIROOT"), "PCI", sizeof("PCI"), path);
        }
      }
    }
    info.path = path;

    get_custom_device_property(hDevInfoSet, devInfo, "PortName", name);
    info.name = name;
    get_reg_property(hDevInfoSet, devInfo, SPDRP_MFG, manufacturer);
    info.manufacturer = manufacturer;
    get_reg_property(hDevInfoSet, devInfo, SPDRP_DEVICEDESC, description);
    info.description = description;

    return &info;
  }
};

struct EnumDataPtr
{
  EnumData* ptr;

  EnumDataPtr() noexcept = default;
  EnumDataPtr(EnumDataPtr& other) noexcept = delete;

  int32_t init()
  {
    reset();
    ptr = reinterpret_cast<EnumData*>(CoTaskMemAlloc(sizeof(EnumData)));
    if (ptr != nullptr)
    {
      ptr = new (ptr) EnumData();
      return ptr->getError();
    }
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  void reset()
  {
    if (ptr != nullptr)
    {
      ptr->~EnumData();
      CoTaskMemFree(ptr);
      ptr = nullptr;
    }
  }

  ~EnumDataPtr() noexcept
  {
    reset();
  }

  EnumData* operator->() noexcept
  {
    return ptr;
  }

  bool operator!() const noexcept
  {
    return ptr == nullptr;
  }
};

/// Thread local managed pointer to store state of queries
thread_local EnumDataPtr s_data;

extern "C"
{
  __declspec(dllexport) int32_t SerialEnum_StartEnumeration()
  {
    return s_data.init();
  }

  __declspec(dllexport) int32_t SerialEnum_Next()
  {
    if (!s_data) return ERROR_INVALID_HANDLE;
    return s_data->next();
  }

  __declspec(dllexport) const SerialDeviceInfo* SerialEnum_GetDeviceInfo()
  {
    if (!s_data) return nullptr;
    return s_data->getDeviceInfo();
  }

  __declspec(dllexport) void SerialEnum_Finish()
  {
    s_data.reset();
  }

  __declspec(dllexport) const char* to_cstring(SerialBusType type) noexcept
  {
    switch (type)
    {
    case SerialBusType::BUS_USB:
      return "USB";
    case SerialBusType::BUS_PCI:
      return "PCI";
    default:
      return "";
    }
  }

}

__declspec(dllexport) std::ostream& operator<<(std::ostream& os, const SerialAPIString& str) noexcept
{
  os.write(str.data, str.length);
  return os;
}

__declspec(dllexport) std::ostream& operator<<(std::ostream& os, SerialBusType type) noexcept
{
  return os << to_cstring(type);
}

