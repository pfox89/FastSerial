
#define API __declspec(dllexport)
#include "SerialEnumeration.h"
#undef API

#include <comdef.h>
#include <initguid.h> // include before devpkey
#include <Devpkey.h>

#include <Cfgmgr32.h>

struct EnumData
{
  EnumData() noexcept = default;

  int32_t init(HANDLE hHeap) noexcept
  {
    _hHeap = hHeap;

    CONFIGRET cr;
    do {
      cr = CM_Get_Device_Interface_List_SizeW(&_listSize, const_cast<LPGUID>(&GUID_DEVINTERFACE_COMPORT), NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
   
      if (cr != CR_SUCCESS) break;

      if (_deviceInterfaceList == nullptr)
      {
        _deviceInterfaceList = reinterpret_cast<PZZWSTR>(HeapAlloc(_hHeap, HEAP_ZERO_MEMORY, _listSize * sizeof(WCHAR)));
       
      }
      else
      {
        LPVOID new_mem = HeapReAlloc(_hHeap, HEAP_ZERO_MEMORY, _deviceInterfaceList, _listSize * sizeof(WCHAR));
        if (new_mem == 0)
        {
          HeapFree(_hHeap, 0, _deviceInterfaceList);
          _deviceInterfaceList = nullptr;
        }
        _deviceInterfaceList = reinterpret_cast<PZZWSTR>(new_mem);
      }

      if (_deviceInterfaceList == nullptr)
      {
        cr = CR_OUT_OF_MEMORY;
        break;
      }

      cr = CM_Get_Device_Interface_ListW(const_cast<LPGUID>(&GUID_DEVINTERFACE_COMPORT),
        NULL,
        _deviceInterfaceList,
        static_cast<ULONG>(_listSize),
        CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

    } while (cr == CR_BUFFER_SMALL);

    _currentInterface = _deviceInterfaceList;

    // Preallocate some memory for buffers
    if (_wbuf == nullptr)
    {
      _wbuf_size = 256;
      _wbuf = static_cast<PZZWSTR>(HeapAlloc(_hHeap, HEAP_ZERO_MEMORY, _wbuf_size));
      if (_wbuf == nullptr) cr = CR_OUT_OF_MEMORY;
    }
    if (_str_buf == nullptr)
    {
      _str_buf_size = 256;
      _str_buf_pos = 0;
      _str_buf = static_cast<LPSTR>(HeapAlloc(_hHeap, HEAP_ZERO_MEMORY, _str_buf_size));
      if (_str_buf == nullptr) cr = CR_OUT_OF_MEMORY;
    }

    if (cr != CR_SUCCESS) return CM_MapCrToWin32Err(cr, ERROR_NOT_SUPPORTED);
    return ERROR_SUCCESS;
  }

  void deinit() noexcept
  {

    if (_deviceInterfaceList != nullptr)
    {
      HeapFree(_hHeap, 0, _deviceInterfaceList);
      _deviceInterfaceList = nullptr;
    }
    if (_wbuf != nullptr)
    {
      HeapFree(_hHeap, 0, _wbuf);
      _wbuf = nullptr;
    }
    if (_str_buf != nullptr)
    {
      HeapFree(_hHeap, 0, _str_buf);
      _str_buf = nullptr;
    }
  }

  ~EnumData() noexcept
  {
    deinit();
  }

  int32_t next(SerialDeviceInfo& info) noexcept
  {
    _currentInterfaceSize = static_cast<DWORD>(wcsnlen_s(_currentInterface, _listSize));
    if (_currentInterfaceSize == 0) return ERROR_NO_MORE_ITEMS;

    _currentDeviceIdLen = sizeof(_currentDeviceId);
    DEVPROPTYPE property_type;
    CONFIGRET cr
      = CM_Get_Device_Interface_PropertyW(_currentInterface,
        &DEVPKEY_Device_InstanceId,
        &property_type,
        (PBYTE)_currentDeviceId,
        &_currentDeviceIdLen,
        0);

    if (cr == CR_SUCCESS)
    {
      if (property_type != DEVPROP_TYPE_STRING) return ERROR_DATATYPE_MISMATCH;

      cr = CM_Locate_DevNodeW(&_device, _currentDeviceId, CM_LOCATE_DEVNODE_NORMAL);
    }

    if (cr != CR_SUCCESS)
    {
      info = { 0 };
      return CM_MapCrToWin32Err(cr, ERROR_NOT_SUPPORTED);
    }

    info.lpath = copybuf(_currentInterface, _currentInterfaceSize);

    auto nextOffset = _currentInterfaceSize + 1;
    _currentInterface = _currentInterface + nextOffset;
    _listSize -= nextOffset;

    DWORD dwSize = 0;
    info.vid = 0;
    info.pid = 0;
    info.type = SerialBusType::BUS_UNKNOWN;

    DEVINST parent;


    cr = get_custom_device_property(L"PortName");
    if(cr == CR_SUCCESS) info.name = copybuf(_wbuf, _wbuf_length);
    
    cr = get_reg_property(_device, CM_DRP_MFG);
    if (cr == CR_SUCCESS) info.manufacturer = copybuf(_wbuf, _wbuf_length);

    cr = get_reg_property(_device, CM_DRP_FRIENDLYNAME);
    if (cr == CR_SUCCESS) info.description = copybuf(_wbuf, _wbuf_length);

    info.type = getBusType();

    // If this isn't on a PCI or USB root, it may be a virtual device referencing a base physical device, so find its parent id
    if (info.type == SerialBusType::BUS_UNKNOWN)
    {
      cr = CM_Get_Parent(&parent, _device, 0);
      if (cr == CR_SUCCESS)
      {
        cr = get_device_id(parent);
        info.type = getBusType();
      }
    }
    else
    {
      parent = _device;
    }

    if (info.type == SerialBusType::BUS_USB)
    {
      const wchar_t* pos = wcsstr(&_currentDeviceId[3], L"VID_");
      if (pos != nullptr)
      {
        wchar_t* next;
        info.vid = static_cast<uint16_t>(wcstoul(pos + 4, &next, 10));
        pos = next;

        pos = wcsstr(pos, L"PID_");
        if (pos != nullptr)
        {
          info.pid = static_cast<uint16_t>(wcstoul(pos + 4, &next, 10));
          pos = next;
        }
      }

      info.path = get_path(parent, L"USBROOT", sizeof(L"USBROOT") / sizeof(wchar_t), L"USB", sizeof(L"USB") / sizeof(wchar_t));
    }
    else if (info.type == SerialBusType::BUS_PCI)
    {
      info.type = SerialBusType::BUS_PCI;
      const wchar_t* pos = wcsstr(&_currentDeviceId[3], L"VEN_");
      if (pos != nullptr)
      {
        wchar_t* next;
        info.vid = static_cast<uint16_t>(wcstoul(pos + 4, &next, 10));
        pos = next;

        pos = wcsstr(pos, L"DEV_");
        if (pos != nullptr)
        {
          info.pid = static_cast<uint16_t>(wcstoul(pos + 4, &next, 16));
          pos = next;
        }
      }

      info.path = get_path(parent, L"PCIROOT", sizeof(L"PCIROOT") / sizeof(wchar_t), L"PCI", sizeof(L"PCI") / sizeof(wchar_t));
    }

    return CM_MapCrToWin32Err(cr, ERROR_NOT_SUPPORTED);
  }

private:

  HANDLE   _hHeap;
  PZZWSTR  _deviceInterfaceList;
  DWORD    _listSize;
  PWSTR    _currentInterface;
  DWORD    _currentInterfaceSize;
  DEVINST  _device;

  WCHAR    _currentDeviceId[MAX_DEVICE_ID_LEN];
  ULONG    _currentDeviceIdLen;

  PZZWSTR _wbuf;
  ULONG   _wbuf_size;
  ULONG   _wbuf_length;

  LPSTR   _str_buf;
  ULONG   _str_buf_pos;
  ULONG   _str_buf_size;


  const char* get_path(DEVINST device, const wchar_t* rootstr, size_t rootstr_size, const wchar_t* hubstr, size_t hubstr_size) noexcept
  {
    CONFIGRET cr = get_reg_property(device, CM_DRP_LOCATION_PATHS);
    if (cr == CR_SUCCESS && _wbuf_length > 0)
    {
      // Multiple paths may be returned separated by null, we only use the first, so look for null terminator
      size_t len = wcsnlen_s(_wbuf, _wbuf_length);
      const wchar_t* pos = wcsstr(_wbuf, rootstr);

      if (pos != nullptr)
      {
        // Reserve plenty of space for this string
        if (strbuf_reserve(len) == false) return nullptr;
        char* out_begin = &_str_buf[_str_buf_pos];
        char* out = out_begin;

        uint16_t path_size = 0;
        *out++ = static_cast<char>(hubstr[0]);
        *out++ = static_cast<char>(hubstr[1]);
        *out++ = static_cast<char>(hubstr[2]);
        *out++ = '/';

        pos += rootstr_size;

        while (std::isdigit(*pos))
        {
          *out++ = static_cast<char>(*pos++);
        }
        while (pos != nullptr && *pos++ == ')' && *pos++ == '#')
        {
          pos = wcsstr(pos, hubstr);
          if (pos != nullptr)
          {
            *out++ = '/';

            pos += hubstr_size;
            while (std::isdigit(*pos))
            {
              *out++ = static_cast<char>(*pos++);
            }
          }
        }

        // Ensure string is null-terminated
        *out++ = '\0';
        _str_buf_pos = static_cast<ULONG>(out - _str_buf);
        return out_begin;
      }
    }
    return nullptr;
  }

  SerialBusType getBusType() noexcept
  {
    if (wcsncmp(_currentDeviceId, L"USB", 3) == 0)
      return SerialBusType::BUS_USB;
    else if (wcsncmp(_currentDeviceId, L"PCI", 3) == 0)
      return SerialBusType::BUS_PCI;
    else return SerialBusType::BUS_UNKNOWN;
  }

  /// @brief Reserve space in internal wide-character buffer used to translate from Windows UCS2 APIs. This does not preserve contents for this temporary buffer
  /// @param size Desired size of buffer, in bytes, including null terminator
  /// @return true if successful, false if failed
  bool resizebuf(DWORD size) noexcept
  {
    if (size > _wbuf_size)
    {
      if (size > UINT16_MAX) size = UINT16_MAX;

      if (_wbuf != nullptr) HeapFree(_hHeap, 0, _wbuf);
      _wbuf = static_cast<PZZWSTR>(HeapAlloc(_hHeap, HEAP_ZERO_MEMORY, size));

      if (_wbuf == nullptr) return false;
      _wbuf_size = static_cast<uint16_t>(size);
      _wbuf_length = static_cast<uint16_t>(size) / sizeof(wchar_t);
    }
    else
    {
      _wbuf_length = static_cast<uint16_t>(size) / sizeof(wchar_t);
      // Ensure buffer is always null-terminated
      _wbuf[_wbuf_length-1] = '\0';
    }
    return true;
  }

  int strbuf_remain() const noexcept
  {
    return _str_buf_size - _str_buf_pos;
  }

  /// @brief Reserve space in output string pool
  /// @param amount Amount of space required
  /// @return true if successful, false if allocation fails
  bool strbuf_reserve(size_t amount) noexcept
  {
    auto remain = _str_buf_size - _str_buf_pos;
    if (amount < remain) return true;

    auto newsize = (_str_buf_size * 3) / 2;
    if (newsize - _str_buf_pos < amount) newsize += static_cast<ULONG>(amount);
   
    LPVOID newbuf = HeapReAlloc(_hHeap, HEAP_ZERO_MEMORY, _str_buf, newsize);

    if (newbuf != nullptr)
    {
      _str_buf = reinterpret_cast<LPSTR>(newbuf);
      _str_buf_size = newsize;
      return true;
    }
    return false;
  }

  /// @brief Copy internal wide string buffer to string buffer and return pointer
  /// @param str Destination string
  /// @return WIN32 error code
  const char* copybuf(const wchar_t* from, ULONG len) noexcept
  {
    int err = 0;

    int count = WideCharToMultiByte(CP_UTF8, 0, from, len, nullptr, 0, nullptr, nullptr);
    if (count == 0) return nullptr;
    if (strbuf_reserve(count) == false) return nullptr;

    LPSTR begin = &_str_buf[_str_buf_pos];
    if (WideCharToMultiByte(CP_UTF8, 0, from, len, begin, count, nullptr, nullptr))
    {
      _str_buf_pos += (count + 1);
      return begin;
    }
    else return nullptr;
  }

  /// @brief Get Device ID from instance
  /// @param deviceInst Instance to get ID for
  /// @return CONFIGRET status code
  CONFIGRET get_device_id(DWORD deviceInst) noexcept
  {
    CONFIGRET ret = CM_Get_Device_ID_Size(&_currentDeviceIdLen, deviceInst, 0);
    if (ret != CR_SUCCESS) return ret;
    return CM_Get_Device_IDW(deviceInst, _currentDeviceId, _currentDeviceIdLen + 1, 0);
  }

  /// @brief Get registry propery from device instance
  /// @param device Device instance to get property for
  /// @param propertyId ID number of property to get, CM_DRP_XXX defined in cfgmgr32.h
  /// @param str String to copy parameter to, or nullptr
  /// @return CONFIGRET status code
  CONFIGRET get_reg_property(DEVINST device, ULONG propertyId) noexcept
  {
    DWORD size = _wbuf_size;
    ULONG type;
    CONFIGRET cr;
    do {
      cr = CM_Get_DevNode_Registry_PropertyW(device, propertyId, &type, _wbuf, &size, 0);
      if (resizebuf(size) == false) return CR_OUT_OF_MEMORY;
    } while (cr == CR_BUFFER_SMALL);

    return cr;
  }

  /// @brief Get custom registry value for device (REG_SZ/string)
  /// @param name Name of registry value, null terminated wide string
  /// @param str Destination to copy value to
  /// @return CONFIGRET status code
  CONFIGRET get_custom_device_property(const wchar_t* name) noexcept
  {
    DWORD size = _wbuf_size;
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
        _wbuf,         // address of data buffer
        &size          // address of data buffer size
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
  __declspec(dllexport) int SerialEnum_StartEnumeration()
  {
    return s_data.init();
  }

  __declspec(dllexport) int SerialEnum_Next(SerialDeviceInfo* info)
  {
    if (s_data.ptr == nullptr || info == nullptr) return ERROR_INVALID_HANDLE;
    return s_data->next(*info);
  }

  __declspec(dllexport) void SerialEnum_Finish()
  {
    s_data.reset();
  }
}

