#pragma once

#include <cstdint>

#ifndef API
#ifdef WIN32
#define API __declspec(dllimport)
#else
#define API
#endif
#endif

/// C compatible string with length
struct SerialAPIString
{
  char*    data;
  uint16_t length;
  uint16_t capacity;
};

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
  SerialAPIString path;
  /// Device name
  SerialAPIString name;
  /// Device description
  SerialAPIString description;
  /// Device manufacturer string
  SerialAPIString manufacturer;
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
  /// \returns Structure containing device info
  API const struct SerialDeviceInfo* SerialEnum_GetDeviceInfo();

  /// Finish enumeration, freeing all associated resources
  API void SerialEnum_Finish();

#ifdef __cplusplus 
}

/// Stream insertion operator to help print SerialAPIStrings
API std::ostream& operator<<(std::ostream& os, const SerialAPIString& str) noexcept;

/// Stream extraction operator to print SerialBusType
API std::ostream& operator<<(std::ostream& os, SerialBusType type) noexcept;

inline std::string to_string(const SerialAPIString& str) noexcept
{
  return std::string(str.data, str.length);
}

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
    const SerialDeviceInfo* info;

    PortIter() noexcept
      : status(SerialEnum_StartEnumeration()), info(nullptr)
    {
      ++(*this);
    }

    PortIter(int) noexcept
      : status(INT32_MAX), info(nullptr)
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
        status = SerialEnum_Next();
        if (status == 0)
          info = SerialEnum_GetDeviceInfo();
        else
          info = nullptr;
      }
     
      return *this;
    }

    const SerialDeviceInfo& operator*() const noexcept
    {
      return *info;
    }
    const SerialDeviceInfo* operator->() const noexcept
    {
      return info;
    }

    bool operator!=(const PortIter&) const noexcept
    {
      return (info != nullptr);
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
