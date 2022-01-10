#include <memory>

#include <comdef.h>
#include <initguid.h> // include before devpkey
#include <Devpkey.h>

#include <Cfgmgr32.h>

#include "SerialEnumeration.h"

/// @brief 
/// @tparam T 
template<class T>
struct process_heap_deleter {
  void operator()(T* ptr) { HeapFree(GetProcessHeap(), 0, ptr); };
};

template<typename T>
using process_heap_ptr = std::unique_ptr <T, process_heap_deleter<T>>;

#pragma warning( push )
#pragma warning( disable : 4200 )
template<class char_type=char>
struct inline_str
{
  unsigned short capacity;
  unsigned short size;
  char_type data[];
};
#pragma warning( pop )

template<class char_type=char>
static process_heap_ptr<inline_str<char_type>> make_inline_str(size_t size) noexcept
{
  using str_type = inline_str<char_type>;
  if (size <= USHRT_MAX)
  {
    void* mem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size + offsetof(str_type, data));
    if (mem != nullptr)
    {
      str_type* p = reinterpret_cast<str_type*>(mem);
      p->capacity = static_cast<unsigned short>(size);
      p->size = 0;
      return process_heap_ptr<str_type>(p);
    }
  }
  return process_heap_ptr<str_type>();
}

template<class char_type=char>
static bool resize_inline_str(process_heap_ptr<inline_str<char_type>>& str, size_t size) noexcept
{
  using str_type = inline_str<char_type>;
  unsigned short oldsize = str->size;
  if (size <= USHRT_MAX)
  {
    void* mem = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, str.get(), size + offsetof(str_type, data));
    if (mem != nullptr)
    {
      str_type* p = reinterpret_cast<str_type*> (mem);
      p->capacity = static_cast<unsigned short>(size);
      p->size = oldsize;
      str.release();
      str = process_heap_ptr<inline_str<char_type>>(p);
      return true;
    }
  }
  return false;
}

using string_buffer_ptr = process_heap_ptr<inline_str<>>;
using wstring_buffer_ptr = process_heap_ptr<inline_str<wchar_t>>;

struct EnumData
{
  HANDLE heap() const noexcept { return _hHeap; }

  /// \brief Initialize enumeration. This function must be called after constructor or deinit() before calling any other functions
  /// \param hHeap Heap to use for allocating memory
  /// \return system_error code
  int32_t init(HANDLE hHeap, unsigned int typeMask) noexcept
  {
    _hHeap = hHeap;
    _typeMask = typeMask;

    CONFIGRET cr;
    do {
      ULONG list_size;
      cr = CM_Get_Device_Interface_List_SizeW(&list_size, const_cast<LPGUID>(&GUID_DEVINTERFACE_COMPORT), NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
   
      if (cr != CR_SUCCESS) break;

      _deviceInterfaceList.reset();
      _deviceInterfaceList = make_inline_str<wchar_t>(list_size * sizeof(WCHAR));
      if (false == _deviceInterfaceList)
      {
        return ERROR_NOT_ENOUGH_MEMORY;
      }

      cr = CM_Get_Device_Interface_ListW(const_cast<LPGUID>(&GUID_DEVINTERFACE_COMPORT),
        NULL,
        _deviceInterfaceList->data,
        list_size,
        CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

      _deviceInterfaceList->size = static_cast<unsigned short>(list_size);
    } while (cr == CR_BUFFER_SMALL);

    _currentInterface = _deviceInterfaceList->data;

    // Preallocate some memory for buffers
    _wbuf = make_inline_str<wchar_t>(500);
    _strbuf = make_inline_str(500);

    if (cr != CR_SUCCESS) return CM_MapCrToWin32Err(cr, ERROR_NOT_SUPPORTED);
    return ERROR_SUCCESS;
  }

  /// \brief Get info for next device in list
  /// \param info reference to struct to store info strings
  /// \return system_error code, or -1 if there are no further devices
  int32_t next(SerialDeviceInfo& info) noexcept
  {
    do {
      _currentInterfaceSize = static_cast<DWORD>(wcsnlen_s(_currentInterface, _deviceInterfaceList->size));
      if (_currentInterfaceSize == 0) return -1;

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

      auto nextOffset = static_cast<unsigned short>(_currentInterfaceSize + 1);
      _currentInterface = _currentInterface + nextOffset;
      _deviceInterfaceList->size -= nextOffset;

      DWORD dwSize = 0;
      info.vid = 0;
      info.pid = 0;
      info.type = SerialBusType::BUS_UNKNOWN;

      DEVINST parent;

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
        if(0 == (_typeMask & static_cast<unsigned>(SerialBusType::BUS_USB)))
          continue;

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
        //info.path = get_path(parent, L"USBROOT", sizeof(L"USBROOT") / sizeof(wchar_t), L"USB", sizeof(L"USB") / sizeof(wchar_t));
  
      }
      else if (info.type == SerialBusType::BUS_PCI)
      {

        if (0 == (_typeMask & static_cast<unsigned>(SerialBusType::BUS_PCI)))
          continue;

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

        //info.path = get_path(parent, L"PCIROOT", sizeof(L"PCIROOT") / sizeof(wchar_t), L"PCI", sizeof(L"PCI") / sizeof(wchar_t));
      }
      else
      {
        // Unknown type, only include on BUS_ANY
        if (_typeMask != static_cast<unsigned>(SerialBusType::BUS_ANY))
          continue;
      }

      cr = get_custom_device_property(L"PortName");
      if (cr == CR_SUCCESS) info.name = copybuf(_wbuf->data, _wbuf->size);

      cr = get_reg_property(_device, CM_DRP_MFG);
      if (cr == CR_SUCCESS) info.manufacturer = copybuf(_wbuf->data, _wbuf->size);

      cr = get_reg_property(_device, CM_DRP_FRIENDLYNAME);
      if (cr == CR_SUCCESS) info.description = copybuf(_wbuf->data, _wbuf->size);

      cr = get_reg_property(parent, CM_DRP_LOCATION_PATHS);
      info.path = parse_path();

      return CM_MapCrToWin32Err(cr, ERROR_NOT_SUPPORTED);
    } while (_currentInterfaceSize > 0);

    return -1;
  }

private:

  HANDLE   _hHeap;
  unsigned _typeMask;
  PWSTR    _currentInterface;
  DWORD    _currentInterfaceSize;
  DEVINST  _device;

  WCHAR    _currentDeviceId[MAX_DEVICE_ID_LEN];
  ULONG    _currentDeviceIdLen;

  wstring_buffer_ptr _deviceInterfaceList;
  wstring_buffer_ptr _wbuf;
  string_buffer_ptr _strbuf;

  const char* parse_path() noexcept
  {
    const wchar_t* pos = _wbuf->data;
    // We probably won't use more than half of the size of the retrieved path in the output, so reserve this much
    if (false == strbuf_reserve(_wbuf->size / 2)) return nullptr;

    char* output_start = &_strbuf->data[_strbuf->size];
    char* output = output_start;

    bool first = true;
    unsigned short size = _strbuf->size;

    while (pos != nullptr)
    {
      if ((_strbuf->capacity - size) < 12)
      {
        // ", Device XX" is 12 bytes, so ensure at least this much is reserved
        if (false == strbuf_reserve(12)) break;
      }

      // Add comma delimeter after first entry
      if (first) first = false;
      else
      {
        *output++ = ',';
        *output++ = ' ';
        size += 2;
      }

      if (_wcsnicmp(pos, L"PCI", 3) == 0)
      {
        if (_wcsnicmp(&pos[3], L"ROOT(", 5) == 0)
        {
          pos += 8;
          memcpy(output, "PCI ", 4);
          output += 4;
          size += (4 + 2);
        }
        else
        {
          pos += 4;
          memcpy(output, "Device ", 7);
          output += 7;
          size += (7 + 2);
          //path->number = static_cast<unsigned short>(strtoul(pos, nullptr, 16) >> 8U);
        }
        if (std::isxdigit(pos[0]))
        {
          *output++ = static_cast<char>(*pos++);
        }
        else break;
        if(std::isxdigit(pos[1]))
        {
          *output++ = static_cast<char>(*pos++);
        }
       
      }
      else if (_wcsnicmp(pos, L"USB", 3) == 0)
      {
        if (_wcsnicmp(&pos[3], L"ROOT(", 5) == 0)
        {
          pos += 8;
          memcpy(output, "USB ", 4);
          output += 4;
          size += 4;
        }
        else
        {
          pos += 4;
          memcpy(output, "Port ", 5);
          output += 5;
          size += 5;
        }

        if (std::isxdigit(pos[0]))
        {
          *output++ = static_cast<char>(*pos++);
        }
        else break;
        if (std::isxdigit(pos[1]))
        {
          *output++ = static_cast<char>(*pos++);
        }
      }
      else
      {
        break;
      }

      // Look for end of this node (delimeter '#')
      const wchar_t* next = wcschr(pos, L'#');
      if (next == nullptr) break;
      pos = ++next;
    }
    *output = '\0';
    ++size;
    _strbuf->size = size;
    return output_start;
  }
  
  /// \brief Get bus type of currently selected device
  /// \return Bus type enumeration
  SerialBusType getBusType() noexcept
  {
    if (wcsncmp(_currentDeviceId, L"USB", 3) == 0)
      return SerialBusType::BUS_USB;
    else if (wcsncmp(_currentDeviceId, L"PCI", 3) == 0)
      return SerialBusType::BUS_PCI;
    else return SerialBusType::BUS_UNKNOWN;
  }

  /// \brief Reserve space in internal wide-character buffer used to translate from Windows UCS2 APIs. This does not preserve contents for this temporary buffer
  /// \param[in] size Desired size of buffer, in bytes, including null terminator
  /// \return true if successful, false if failed
  bool resizebuf(DWORD size) noexcept
  {
    if (size > _wbuf->capacity)
    {
      _wbuf.reset();
      _wbuf = make_inline_str<wchar_t>(size);
      if (false == _wbuf) return false;
      _wbuf->size = static_cast<unsigned short>(size);
    }
    else
    {
      // Size includes null terminator
      _wbuf->size = (static_cast<unsigned short>(size) / sizeof(wchar_t));
      // Ensure buffer is always null-terminated
      _wbuf->data[_wbuf->size-1] = '\0';
    }
    return true;
  }

  /// \brief Reserve space in output string pool
  /// \param amount Amount of space required
  /// \return true if successful, false if allocation fails
  bool strbuf_reserve(size_t amount) noexcept
  {
    size_t capacity = _strbuf->capacity;
    size_t size = _strbuf->size;
    size_t remain = capacity - size;
    if (amount < remain) return true;

    auto newsize = (capacity * 3) / 2;
    if (newsize - remain < amount) newsize += static_cast<ULONG>(amount);
   
    return resize_inline_str(_strbuf, newsize);
  }

  /// \brief Copy internal wide string buffer to string buffer and return pointer
  /// \param str Destination string
  /// \return WIN32 error code
  const char* copybuf(const wchar_t* from, ULONG len) noexcept
  {
    int err = 0;

    int count = WideCharToMultiByte(CP_UTF8, 0, from, len, nullptr, 0, nullptr, nullptr);
    if (count == 0) return nullptr;
    if (strbuf_reserve(count) == false) return nullptr;

    auto pos = _strbuf->size;
    LPSTR begin = &_strbuf->data[pos];
    if (WideCharToMultiByte(CP_UTF8, 0, from, len, begin, count, nullptr, nullptr))
    {
      pos += (count + 1);
      _strbuf->size = pos;
      return begin;
    }
    else return nullptr;
  }

  /// \brief Get Device ID from instance
  /// \param deviceInst Instance to get ID for
  /// \return CONFIGRET status code
  CONFIGRET get_device_id(DWORD deviceInst) noexcept
  {
    CONFIGRET ret = CM_Get_Device_ID_Size(&_currentDeviceIdLen, deviceInst, 0);
    if (ret != CR_SUCCESS) return ret;
    return CM_Get_Device_IDW(deviceInst, _currentDeviceId, _currentDeviceIdLen + 1, 0);
  }

  /// \brief Get registry propery from device instance
  /// \param device Device instance to get property for
  /// \param propertyId ID number of property to get, CM_DRP_XXX defined in cfgmgr32.h
  /// \param str String to copy parameter to, or nullptr
  /// \return CONFIGRET status code
  CONFIGRET get_reg_property(DEVINST device, ULONG propertyId) noexcept
  {
    DWORD size = _wbuf->capacity;
    ULONG type;
    CONFIGRET cr;
    do {
      cr = CM_Get_DevNode_Registry_PropertyW(device, propertyId, &type, _wbuf->data, &size, 0);
      if (resizebuf(size) == false) return CR_OUT_OF_MEMORY;
    } while (cr == CR_BUFFER_SMALL);

    return cr;
  }

  /// \brief Get custom registry value for device (REG_SZ/string)
  /// \param name Name of registry value, null terminated wide string
  /// \param str Destination to copy value to
  /// \return CONFIGRET status code
  CONFIGRET get_custom_device_property(const wchar_t* name) noexcept
  {
    DWORD size = _wbuf->capacity;
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
        _wbuf->data,   // address of data buffer
        &size          // address of data buffer size
      );
      if (false == resizebuf(size)) return CR_OUT_OF_MEMORY;

    } while (err == ERROR_MORE_DATA);

    _wbuf->size = static_cast<unsigned short>(size);

    if (cr == CR_SUCCESS && err != ERROR_SUCCESS)
      cr = CR_REGISTRY_ERROR;

    err = RegCloseKey(deviceKey);

    if (cr == CR_SUCCESS && err != ERROR_SUCCESS)
      cr = CR_REGISTRY_ERROR;

    return cr;
  }
};

/// Manages pointer to enumeration object to minimize memory consumption when not in use
struct EnumDataPtr
{
  EnumData* ptr;

  EnumDataPtr() noexcept = default;
  EnumDataPtr(EnumDataPtr& other) noexcept = delete;

  /// \brief Allocate memory and initialize enumeration
  /// \return system_error code
  int init(unsigned int typeMask) noexcept
  {
    reset();
    HANDLE hHeap = GetProcessHeap();
    ptr = reinterpret_cast<EnumData*>(HeapAlloc(hHeap, 0, sizeof(EnumData)));

    int err;
    if (ptr == nullptr)
    {
      err = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
      // Placement new to initialize object
      ptr = new (ptr) EnumData();
      err = ptr->init(hHeap, typeMask);

      // If we ran out of memory, free all resources
      if (err == ERROR_NOT_ENOUGH_MEMORY) reset();
    }
    return err;
  }

  /// \brief Deinitialize and free resources
  void reset() noexcept
  {
    if (ptr != nullptr)
    {
      HANDLE heap = ptr->heap();
      // Call destructor to free managed resources
      ptr->~EnumData();
      HeapFree(heap, 0, ptr);
      ptr = nullptr;
    }
  }

  /// Destructor deinitializes all resources
  ~EnumDataPtr() noexcept
  {
    reset();
  }
};

/// Thread local managed pointer to store state of queries
thread_local EnumDataPtr s_data;

#if defined(EXPORTING_SERIALENUM)
#  define DECLSPEC __declspec(dllexport)
#else
#  define DECLSPEC
#endif

extern "C"
{
  DECLSPEC int SerialEnum_StartEnumeration(unsigned int typeMask)
  {
    return s_data.init(typeMask);
  }

  DECLSPEC int SerialEnum_Next(SerialDeviceInfo* info)
  {
    if (s_data.ptr == nullptr || info == nullptr) return ERROR_INVALID_HANDLE;
    return s_data.ptr->next(*info);
  }

  DECLSPEC void SerialEnum_Finish()
  {
    s_data.reset();
  }
}
