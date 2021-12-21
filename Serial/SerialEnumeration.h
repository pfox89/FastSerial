#pragma once

#ifndef API
#ifdef WIN32
#define API __declspec(dllimport)
#else
#define API
#endif
#endif

/// Possible types of bus this device can be connected to

#ifdef __cplusplus 
enum class SerialBusType
#else
enum SerialBusType
#endif
{
  BUS_UNKNOWN,
  BUS_USB,
  BUS_PCI
};


/// Information about device
struct SerialDeviceInfo
{
  /// Logical device path (e.g. interface)
  const char* lpath;
  /// Physical device path
  const char* path;
  /// Device name
  const char* name;
  /// Device description
  const char* description;
  /// Device manufacturer string
  const char* manufacturer;
  /// Numeric vendor id (VID on USB, VEN on PCI)
  unsigned short vid;
  /// Numeric device id (PID on USB, DEV on PCI)
  unsigned short pid;
  /// Type of bus device is connected to
  SerialBusType type;
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
  API int SerialEnum_StartEnumeration();

  /// Gets info for next device
  /// \returns Status/error code of system_category type
  API int SerialEnum_Next(SerialDeviceInfo* info);

  /// Finish enumeration, freeing all associated resources
  API void SerialEnum_Finish();

#ifdef __cplusplus 
}

/// Stream extraction operator to print SerialBusType
API std::ostream& operator<<(std::ostream& os, SerialBusType type) noexcept;

API std::ostream& operator<<(std::ostream& os, const SerialDeviceInfo& port) noexcept;

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
        status = SerialEnum_Next(&info);
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
      return (status == 0);
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
