
#define API __declspec(dllexport)
#include "SerialEnumeration.h"
#undef API

#include <ostream>
#include <comdef.h>
#include <initguid.h>   // include before devpropdef.h
#include <Devpkey.h>
#include <Devguid.h>
#include <initguid.h>

#include <Cfgmgr32.h>

struct WideStringBuffer
{
  PZZWSTR  _data;
  uint16_t _size;
  uint16_t _capacity;
};

struct EnumData
{
private:
  SerialAPIString  _path;
  SerialAPIString  _description;
  SerialAPIString  _name;
  SerialAPIString  _manufacturer;
  SerialDeviceInfo _info;

  HANDLE           _hHeap;
  PZZWSTR          _deviceInterfaceList;
  size_t           _listSize;
  PWSTR            _currentInterface;
  size_t           _currentInterfaceSize;
  DEVINST          _device;

  WCHAR             _currentDeviceId[MAX_DEVICE_ID_LEN];
  DWORD             _currentDeviceIdLen;

  WideStringBuffer  _wbuf;

  CONFIGRET resize(DWORD size, SerialAPIString& string) noexcept
  {
    if (size > string.capacity)
    {
      if (size > UINT16_MAX) size = UINT16_MAX;

      void* newbuf;
      if (string.data == nullptr) newbuf = HeapAlloc(_hHeap, 0, size);
      else newbuf = HeapReAlloc(_hHeap, 0, string.data, size);

      if (newbuf == nullptr) return CR_OUT_OF_MEMORY;
      string.data = reinterpret_cast<char*>(newbuf);
      string.capacity = static_cast<uint16_t>(size);
      string.length = static_cast<uint16_t>(size - 1);
    }
    else
    {
      string.length = static_cast<uint16_t>(size - 1);
    }
    return CR_SUCCESS;
  }

  CONFIGRET resizebuf(DWORD size) noexcept
  {
    if (size > _wbuf._capacity)
    {
      if (size > UINT16_MAX) size = UINT16_MAX;

      void* newbuf;
      if (_wbuf._data == nullptr) newbuf = HeapAlloc(_hHeap, HEAP_ZERO_MEMORY, size);
      else newbuf = HeapReAlloc(_hHeap, HEAP_ZERO_MEMORY, _wbuf._data, size);

      if (newbuf == nullptr) return CR_OUT_OF_MEMORY;
      _wbuf._data = reinterpret_cast<wchar_t*>(newbuf);
      _wbuf._capacity = static_cast<uint16_t>(size);
      _wbuf._size = static_cast<uint16_t>(size - 1) / sizeof(wchar_t);
    }
    else
    {
      _wbuf._size = static_cast<uint16_t>(size - 1) / sizeof(wchar_t);
    }
    return CR_SUCCESS;
  }

  DWORD copybuf(SerialAPIString& str) noexcept
  {
    int err;

    err = WideCharToMultiByte(CP_UTF8, 0, _wbuf._data, _wbuf._size, nullptr, 0, nullptr, nullptr);
    if (err == 0) return GetLastError();
    resize(err+1, str);
    err = WideCharToMultiByte(CP_UTF8, 0, _wbuf._data, _wbuf._size, str.data, str.capacity, nullptr, nullptr);
    return 0;
  }

  void reset(SerialAPIString& str) noexcept
  {
    if (str.data != nullptr)
    {
      HeapFree(_hHeap, 0, str.data);
      str.data = nullptr;
      str.length = 0;
    }
  }

  CONFIGRET get_device_id(DWORD deviceInst) noexcept
  {
    CONFIGRET ret = CM_Get_Device_ID_Size(&_currentDeviceIdLen, deviceInst, 0);
    if (ret != CR_SUCCESS) return ret;
    return CM_Get_Device_IDW(deviceInst, _currentDeviceId, _currentDeviceIdLen + 1, 0);
  }

  CONFIGRET get_reg_property(DEVINST device, ULONG propertyId) noexcept
  {
    DWORD size = _wbuf._capacity;
    ULONG type;
    CONFIGRET cr;
    do {
      cr = CM_Get_DevNode_Registry_PropertyW(device, propertyId, &type, _wbuf._data, &size, 0);
      resizebuf(size);
    } while (cr == CR_BUFFER_SMALL);

    return cr;
  }

  CONFIGRET get_custom_device_property(const wchar_t* name) noexcept
  {
    DWORD size = _wbuf._capacity;
    HKEY deviceKey;
    CONFIGRET cr = CM_Open_DevNode_Key(_device, KEY_QUERY_VALUE, 0, RegDisposition_OpenExisting, &deviceKey, CM_REGISTRY_HARDWARE);
    if (cr != CR_SUCCESS) return cr;
    DWORD err;
    do {
      err = RegGetValueW(
        deviceKey,     // handle of key to query
        nullptr,       // No subkey
        name,          // Value name to query
        RRF_RT_REG_SZ, // Type of value to receieve
        nullptr,       // We only expect a REG_SZ key, so we don't need it reported back
        _wbuf._data, // address of data buffer
        &size        // address of data buffer size
      );
      resizebuf(size);

    } while (err == ERROR_MORE_DATA);

    if (cr == CR_SUCCESS && err != ERROR_SUCCESS)
      cr = CR_REGISTRY_ERROR;

    err = RegCloseKey(deviceKey);

    if (cr == CR_SUCCESS && err != ERROR_SUCCESS)
      cr = CR_REGISTRY_ERROR;

    return cr;
  }

public:
  EnumData() noexcept = default;

  CONFIGRET init(HANDLE hHeap) noexcept
  {
    _hHeap = hHeap;
    CONFIGRET cr = CR_SUCCESS;
    do {
      DWORD size;
      cr = CM_Get_Device_Interface_List_SizeW(&size, const_cast<LPGUID>(&GUID_DEVINTERFACE_COMPORT), NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
      _listSize = size;
      if (cr == CR_SUCCESS)
      {
        if (_deviceInterfaceList == nullptr)
        {
          _deviceInterfaceList = reinterpret_cast<PZZWSTR>(HeapAlloc(_hHeap, HEAP_ZERO_MEMORY, _listSize * sizeof(WCHAR)));
        }
        else
        {
          LPVOID new_mem = HeapReAlloc(_hHeap, HEAP_ZERO_MEMORY, _deviceInterfaceList, _listSize * sizeof(WCHAR));
          if (new_mem == 0) cr = CR_OUT_OF_MEMORY;
          _deviceInterfaceList = reinterpret_cast<PZZWSTR>(new_mem);
        }

        if (_deviceInterfaceList != nullptr)
        {
          CM_Get_Device_Interface_ListW(const_cast<LPGUID>(&GUID_DEVINTERFACE_COMPORT),
            NULL,
            _deviceInterfaceList,
            static_cast<ULONG>(_listSize),
            CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
        }
        else
        {
          cr = CR_OUT_OF_MEMORY;
        }
      }
    } while (cr == CR_BUFFER_SMALL);
    _currentInterface = _deviceInterfaceList;
    if (_currentInterface != nullptr)
      _currentInterfaceSize = wcsnlen_s(_currentInterface, _listSize);

    return getInstance();
  }
  ~EnumData() noexcept
  {
    reset(_path);
    reset(_description);
    reset(_name);
    reset(_manufacturer);
    if (_deviceInterfaceList != nullptr)
    {
      HeapFree(_hHeap, 0, _deviceInterfaceList);
      _deviceInterfaceList = nullptr;
    }
    if (_wbuf._data != nullptr)
    {
      HeapFree(_hHeap, 0, _wbuf._data);
      _wbuf._data = 0;
      _wbuf._capacity = 0;
    }
  }

  CONFIGRET getInstance() noexcept
  {

    ULONG property_size = sizeof(_currentDeviceId);
    DEVPROPTYPE property_type;
    CONFIGRET cr
      = CM_Get_Device_Interface_PropertyW(_currentInterface,
        &DEVPKEY_Device_InstanceId,
        &property_type,
        (PBYTE)_currentDeviceId,
        &property_size,
        0);

    if (cr != CR_SUCCESS) return cr;

    if (property_type != DEVPROP_TYPE_STRING)
      return CR_INVALID_PROPERTY;

    cr = CM_Locate_DevNodeW(&_device, _currentDeviceId, CM_LOCATE_DEVNODE_NORMAL);

    return cr;
  }

  CONFIGRET next() noexcept
  {
    auto nextOffset = _currentInterfaceSize + 1;
    _currentInterface = _currentInterface + nextOffset;
    _currentInterfaceSize = wcsnlen_s(_currentInterface, _listSize);
    _listSize -= nextOffset;

    // TODO: what should we report at the end of the list?
    if (_currentInterfaceSize == 0) return CR_NO_MORE_HW_PROFILES;

    return getInstance();
  }

  CONFIGRET get_path(DEVINST device, const wchar_t* rootstr, size_t rootstr_size, const wchar_t* hubstr, size_t hubstr_size, SerialAPIString& str) noexcept
  {
    CONFIGRET cr = get_reg_property(device, CM_DRP_LOCATION_PATHS);
    if (cr == CR_SUCCESS && _wbuf._size > 0)
    {
      const wchar_t* pos = wcsstr(_wbuf._data, rootstr);

      if (pos != nullptr)
      {
        const wchar_t* posend = wcschr(pos, '\0');

        // Reserve plenty of space for this string
        cr = resize(static_cast<uint16_t>(posend - pos), str);
       
        if (cr != CR_SUCCESS) return cr;

        uint16_t path_size = 0;
        str.data[path_size++] = static_cast<char>(hubstr[0]);
        str.data[path_size++] = static_cast<char>(hubstr[1]);
        str.data[path_size++] = static_cast<char>(hubstr[2]);
        str.data[path_size++] = '/';
         
        pos += rootstr_size;
        while (std::isdigit(*pos))
        {
          str.data[path_size++] = static_cast<char>(*pos++);
        }
        while (pos != nullptr && *pos++ == ')' && *pos++ == '#')
        {
          pos = wcsstr(pos, hubstr);
          if (pos != nullptr)
          {
            str.data[path_size++] = '/';

            pos += hubstr_size;
            while (std::isdigit(*pos))
            {
              str.data[path_size++] = static_cast<char>(*pos++);
            }
          }
          // Ensure string is null-terminated and resize
          str.data[path_size++] = '\0';
          cr = resize(path_size, str);
        }
      }
    }
    return cr;
  }


  const SerialDeviceInfo* getDeviceInfo() noexcept
  {
    DWORD dwSize = 0;
    _info.vid = 0;
    _info.pid = 0;
    _info.type = SerialBusType::BUS_UNKNOWN;

    //if (get_device_id(_path, _device) == CR_SUCCESS)
    //{
    DEVINST parent;

    // If this isn't on a PCI or USB root, it may be a virtual device referencing a base physical device, so find its parent id
    if (wcsncmp(_currentDeviceId, L"USB", 3) != 0 && wcsncmp(_currentDeviceId, L"PCI", 3) != 0)
    {
      if (CM_Get_Parent(&parent, _device, 0) == CR_SUCCESS)
      {
        get_device_id(parent);
      }
    }
    else
    {
      parent = _device;
    }

    if (_currentDeviceIdLen > 2)
    {
      if (wcsncmp(_currentDeviceId, L"USB", 3) == 0)
      {
        _info.type = SerialBusType::BUS_USB;

        {
          const wchar_t* pos = wcsstr(&_currentDeviceId[3], L"VID_");
          if (pos != nullptr)
          {
            wchar_t* next;
            _info.vid = static_cast<uint16_t>(wcstoul(pos + 4, &next, 10));
            pos = next;

            pos = wcsstr(pos, L"PID_");
            if (pos != nullptr)
            {
              _info.pid = static_cast<uint16_t>(wcstoul(pos + 4, &next, 10));
              pos = next;
            }
          }
        }

        get_path(parent, L"USBROOT", sizeof(L"USBROOT") / sizeof(wchar_t), L"USB", sizeof(L"USB") / sizeof(wchar_t), _path);

      }
      else if (wcsncmp(_currentDeviceId, L"PCI", 3) == 0)
      {
        _info.type = SerialBusType::BUS_PCI;
        const wchar_t* pos = wcsstr(&_currentDeviceId[3], L"VEN_");
        if (pos != nullptr)
        {
          wchar_t* next;
          _info.vid = static_cast<uint16_t>(wcstoul(pos + 4, &next, 10));
          pos = next;

          pos = wcsstr(pos, L"DEV_");
          if (pos != nullptr)
          {
            _info.pid = static_cast<uint16_t>(wcstoul(pos + 4, &next, 16));
            pos = next;
          }
        }

        get_path(parent, L"PCIROOT", sizeof(L"PCIROOT") / sizeof(wchar_t), L"PCI", sizeof(L"PCI") / sizeof(wchar_t), _path);
      }
    }
    //}

    _info.path = _path;

    get_custom_device_property(L"PortName");
    copybuf(_name);
    _info.name = _name;

    get_reg_property(_device, CM_DRP_MFG);
    copybuf(_manufacturer);
    _info.manufacturer = _manufacturer;

    get_reg_property(_device, CM_DRP_FRIENDLYNAME);
    copybuf(_description);
    _info.description = _description;

    return &_info;
  }
};

struct EnumDataPtr
{
  EnumData* ptr;
  HANDLE hHeap;

  EnumDataPtr() noexcept = default;
  EnumDataPtr(EnumDataPtr& other) noexcept = delete;

  CONFIGRET init() noexcept
  {
    reset();
    hHeap = GetProcessHeap();
    ptr = reinterpret_cast<EnumData*>(HeapAlloc(hHeap, 0, sizeof(EnumData)));

    CONFIGRET cr;
    if (ptr == nullptr)
    {
      cr = CR_OUT_OF_MEMORY;
    }
    else
    {
      // Placement new to initialize object
      ptr = new (ptr) EnumData();
      cr = ptr->init(hHeap);
    }
    return cr;
  }

  void reset() noexcept
  {
    if (ptr != nullptr)
    {
      ptr->~EnumData();
      HeapFree(hHeap, 0, ptr);
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
    return CM_MapCrToWin32Err(s_data.init(), ERROR_NOT_SUPPORTED);
  }

  __declspec(dllexport) int32_t SerialEnum_Next()
  {
    if (!s_data) return ERROR_INVALID_HANDLE;
    return CM_MapCrToWin32Err(s_data->next(), ERROR_NOT_SUPPORTED);
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

  __declspec(dllexport) const char* to_cstring(SerialBusType type)
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

