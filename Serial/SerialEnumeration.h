#pragma once

#include <cstdint>

#ifndef API
#ifdef WIN32
#define API __declspec(dllimport)
#else
#define API
#endif
#endif


/// Possible types of bus this device can be connected to
enum class SerialBusType
{
  BUS_UNKNOWN,
  BUS_USB,
  BUS_PCI
};

/// Information about device
struct SerialDeviceInfo
{
  /// Physical device path
  const char* path;
  /// Device name
  const char* name;
  /// Device description
  const char* description;
  /// Device manufacturer string
  const char* manufacturer;
  /// Type of bus device is connected to
  SerialBusType type;
  /// Numeric vendor id (VID on USB, VEN on PCI)
  uint16_t vid;
  /// Numeric device id (PID on USB, DEV on PCI)
  uint16_t pid;
};

#ifdef __cplusplus 

#include <iosfwd>
#include <string>

extern "C" {
#endif

  /// Get String describing SerialBusType
  API const char* to_cstring(SerialBusType type);

  /// Start enumerating devices
  /// \returns Status/error code of system_category type
  /// \remarks Always call SerialEnum_Finish after this function to free resources allocated
  API int32_t SerialEnum_StartEnumeration();

  /// Get next device
  /// \returns Status/error code of system_category type
  API int32_t SerialEnum_Next();

  /// Gets info for current device
  /// \param info Pointer to structure to store info
  /// \returns Structure containing device info
  API int32_t SerialEnum_GetDeviceInfo(SerialDeviceInfo* info);

  /// Finish enumeration, freeing all associated resources
  API void SerialEnum_Finish();

#ifdef __cplusplus 
}


/// Stream extraction operator to print SerialBusType
API std::ostream& operator<<(std::ostream& os, SerialBusType type) noexcept;

inline std::string to_string(SerialBusType type) noexcept
{
  return std::string(to_cstring(type));
}
#endif

#ifdef __cplusplus

namespace Serial
{
  struct PortIter
  {
    int32_t status;
    SerialDeviceInfo info;

    PortIter() noexcept
      : status(SerialEnum_StartEnumeration()), info{}
    {
      ++(*this);
    }

    PortIter(int) noexcept
      : status(INT32_MAX), info{}
    {}

    PortIter(const PortIter&) = delete;
    PortIter(PortIter&& other) noexcept
      : status(other.status), info(other.info)
    {
      other.status = INT32_MAX;
    }

    PortIter& operator++() noexcept
    {
      if (status == 0)
      {
        
        status = SerialEnum_GetDeviceInfo(&info);
         
        status = SerialEnum_Next();
      }
     
      return *this;
    }

    const SerialDeviceInfo& operator*() const noexcept
    {
      return info;
    }
    const SerialDeviceInfo* operator->() const noexcept
    {
      return &info;
    }

    bool operator!=(const PortIter&) const noexcept
    {
      return (info.name != nullptr);
    }

    ~PortIter() noexcept
    {
      if (status != INT32_MAX)
        SerialEnum_Finish();
    }
  };

  struct PortInfo
  {
    PortIter begin() const noexcept
    {
      return PortIter();
    }
    PortIter end() const noexcept
    {
      return PortIter(INT32_MAX);
    }
  };

  static constexpr PortInfo ports;
}

#endif
