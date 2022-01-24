---
title: Serial::Enum

---

# Serial::Enum



 [More...](#detailed-description)


`#include <SerialEnumeration.hpp>`

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[Enum](/Documentation/Classes/struct_serial_1_1_enum/#function-enum)**() =default |
| | **[Enum](/Documentation/Classes/struct_serial_1_1_enum/#function-enum)**(const [Enum](/Documentation/Classes/struct_serial_1_1_enum/) & other) =delete |
| | **[Enum](/Documentation/Classes/struct_serial_1_1_enum/#function-enum)**([Enum](/Documentation/Classes/struct_serial_1_1_enum/) && other)<br>Move constructor allows moving pointer.  |
| int | **[getInfoFor](/Documentation/Classes/struct_serial_1_1_enum/#function-getinfofor)**([DeviceInfo](/Documentation/Classes/struct_serial_1_1_device_info/) & info, const void * id)<br>Get info for given device ID.  |
| int | **[begin](/Documentation/Classes/struct_serial_1_1_enum/#function-begin)**(unsigned int type_mask =static_cast< unsigned int >(BusType::BUS_ANY))<br>Start enumerating devices of given types.  |
| int | **[next](/Documentation/Classes/struct_serial_1_1_enum/#function-next)**([DeviceInfo](/Documentation/Classes/struct_serial_1_1_device_info/) & info)<br>Get info for next device in enumeration.  |
| void | **[clear](/Documentation/Classes/struct_serial_1_1_enum/#function-clear)**()<br>Free all resources associated with enumeration. This will invalidate [DeviceInfo](/Documentation/Classes/struct_serial_1_1_device_info/) strings.  |
| | **[~Enum](/Documentation/Classes/struct_serial_1_1_enum/#function-~enum)**() |

## Detailed Description

```cpp
struct Serial::Enum;
```


Serial Enumerator class, which manages resource associated with serial device enumeration In general, [begin()](/Documentation/Classes/struct_serial_1_1_enum/#function-begin) must be called to initate enumeration, and [next()](/Documentation/Classes/struct_serial_1_1_enum/#function-next) called to get subsequent devices until it returns <= 0 

## Public Functions Documentation

### function Enum

```cpp
Enum() =default
```


### function Enum

```cpp
Enum(
    const Enum & other
) =delete
```


### function Enum

```cpp
inline Enum(
    Enum && other
)
```

Move constructor allows moving pointer. 

### function getInfoFor

```cpp
int getInfoFor(
    DeviceInfo & info,
    const void * id
)
```

Get info for given device ID. 

**Parameters**: 

  * **info** Reference of struct to store info of device 
  * **id** ID of device. This will be a utf-8 encoded syspath on Linux, or a Windows device ID encoded as LPWSTR 


**Return**: == 0 No device found > 0 [Device](/Documentation/Classes/struct_serial_1_1_device/) info retrieved successfully < 0 Error occurred, std::error_code(-return, system_category()) for details. 

### function begin

```cpp
int begin(
    unsigned int type_mask =static_cast< unsigned int >(BusType::BUS_ANY)
)
```

Start enumerating devices of given types. 

**Parameters**: 

  * **type_mask** Bitwise combination of any BusType values defining what kind of devices to include 


**Return**: == 0 Successful < 0 Error occurred, std::error_code(-return, system_category()) for details. 

### function next

```cpp
int next(
    DeviceInfo & info
)
```

Get info for next device in enumeration. 

**Parameters**: 

  * **info** Reference of struct to store info of device 


**Return**: == 0 No more devices found > 0 [Device](/Documentation/Classes/struct_serial_1_1_device/) info retrieved successfully < 0 Error occurred, std::error_code(-return, system_category()) for details. 

**Note**: All strings are bound to the current enumeration state, so strings in [DeviceInfo](/Documentation/Classes/struct_serial_1_1_device_info/) should be copied before calling any other member functions if they will be needed later. 

### function clear

```cpp
void clear()
```

Free all resources associated with enumeration. This will invalidate [DeviceInfo](/Documentation/Classes/struct_serial_1_1_device_info/) strings. 

### function ~Enum

```cpp
inline ~Enum()
```


-------------------------------

Updated on 2022-01-24 at 14:10:27 -0500