#pragma once

#if defined(_WIN32)
#  if defined(EXPORTING_SERIAL)
#    define DECLSPEC __declspec(dllexport)
#  else
#    define DECLSPEC __declspec(dllimport)
#  endif
#else // non windows
#  define DECLSPEC [[gnu::visibility("default")]]
#endif

#ifdef __cplusplus 
/// Possible types of bus this device can be connected to
enum class SerialBusType
#else
enum SerialBusType
#endif
{
  BUS_UNKNOWN,
  BUS_USB,
  BUS_PCI,
  BUS_ISA,
  BUS_PLATFORM
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

  /// Start enumerating devices
  /// \returns Status/error code of system_category type
  /// \remarks Always call SerialEnum_Finish after this function to free resources allocated
  DECLSPEC int SerialEnum_StartEnumeration();

  /// Gets info for next device
  /// \returns Status/error code of system_category type
  DECLSPEC int SerialEnum_Next(SerialDeviceInfo* info);

  /// Finish enumeration, freeing all associated resources
  DECLSPEC void SerialEnum_Finish();

  /// Get String describing SerialBusType
  DECLSPEC const char* to_cstring(SerialBusType type);

#ifdef __cplusplus 
}

/// Stream insertion operator to print SerialBusType
DECLSPEC std::ostream& operator<<(std::ostream& os, SerialBusType type) noexcept;

/// Stream insertion operator to pretty-print serial device information
DECLSPEC std::ostream& operator<<(std::ostream& os, const SerialDeviceInfo& port) noexcept;

inline std::string to_string(SerialBusType type) noexcept
{
  return std::string(to_cstring(type));
}
#endif

#undef DECLSPEC

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

  /// Convenience enumerator object
  static constexpr PortInfo ports;
}

#endif
