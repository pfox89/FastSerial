---
title: Serial

---

# Serial



## Namespaces

| Name           |
| -------------- |
| **[Serial::pci](/Documentation/Namespaces/namespace_serial_1_1pci/)**  |
| **[Serial::platform](/Documentation/Namespaces/namespace_serial_1_1platform/)**  |
| **[Serial::pnp](/Documentation/Namespaces/namespace_serial_1_1pnp/)**  |
| **[Serial::usb](/Documentation/Namespaces/namespace_serial_1_1usb/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Serial::Device](/Documentation/Classes/struct_serial_1_1_device/)**  |
| struct | **[Serial::DeviceInfo](/Documentation/Classes/struct_serial_1_1_device_info/)** <br>Information about device.  |
| struct | **[Serial::Enum](/Documentation/Classes/struct_serial_1_1_enum/)**  |
| struct | **[Serial::Frame](/Documentation/Classes/struct_serial_1_1_frame/)** <br>Read buffer to accumulate reads so we can ensure we read a minimum amount of data.  |
| struct | **[Serial::FrameBuffer](/Documentation/Classes/struct_serial_1_1_frame_buffer/)** <br>Encapsulates a buffer to receive a frame, with a certain maximum size. This class allows consecutive polling of the device until desired amount of data has been received.  |
| struct | **[Serial::PortInfo](/Documentation/Classes/struct_serial_1_1_port_info/)** <br>Constexpr range template to allow enumerator to be used as a range.  |
| struct | **[Serial::PortIter](/Documentation/Classes/struct_serial_1_1_port_iter/)** <br>Iterator to iterate over ports enumerator.  |

## Types

|                | Name           |
| -------------- | -------------- |
| enum class| **[BusType](/Documentation/Namespaces/namespace_serial/#enum-bustype)** { BUS_UNKNOWN = 0, BUS_USB = 0x1, BUS_PCI = 0x2, BUS_PNP = 0x4, BUS_PLATFORM = 0x8, BUS_ANY = BUS_USB | BUS_PCI | BUS_PNP | BUS_PLATFORM}<br>Possible types of bus this device can be connected to.  |

## Functions

|                | Name           |
| -------------- | -------------- |
| const char * | **[to_cstring](/Documentation/Namespaces/namespace_serial/#function-to-cstring)**(BusType type) |
| std::string | **[to_string](/Documentation/Namespaces/namespace_serial/#function-to-string)**(BusType type) |
| std::ostream & | **[operator<<](/Documentation/Namespaces/namespace_serial/#function-operator<<)**(std::ostream & os, BusType type) |
| std::ostream & | **[operator<<](/Documentation/Namespaces/namespace_serial/#function-operator<<)**(std::ostream & os, const [DeviceInfo](/Documentation/Classes/struct_serial_1_1_device_info/) & port)<br>Stream insertion operator to pretty-print serial device information.  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| constexpr [PortInfo](/Documentation/Classes/struct_serial_1_1_port_info/)< BusType::BUS_ANY > | **[ports](/Documentation/Namespaces/namespace_serial/#variable-ports)** <br>Enuemrator object for all ports Use begin() and end() functions to get iterators to port range, or use in range-based for loop, e.g. `for(auto& port : ports)` |

## Types Documentation

### enum BusType

| Enumerator | Value | Description |
| ---------- | ----- | ----------- |
| BUS_UNKNOWN | 0|   |
| BUS_USB | 0x1|   |
| BUS_PCI | 0x2|   |
| BUS_PNP | 0x4|   |
| BUS_PLATFORM | 0x8|   |
| BUS_ANY | BUS_USB | BUS_PCI | BUS_PNP | BUS_PLATFORM|   |



Possible types of bus this device can be connected to. 


## Functions Documentation

### function to_cstring

```cpp
const char * to_cstring(
    BusType type
)
```


### function to_string

```cpp
inline std::string to_string(
    BusType type
)
```


### function operator<<

```cpp
std::ostream & operator<<(
    std::ostream & os,
    BusType type
)
```


### function operator<<

```cpp
std::ostream & operator<<(
    std::ostream & os,
    const DeviceInfo & port
)
```

Stream insertion operator to pretty-print serial device information. 


## Attributes Documentation

### variable ports

```cpp
static constexpr PortInfo< BusType::BUS_ANY > ports;
```

Enuemrator object for all ports Use begin() and end() functions to get iterators to port range, or use in range-based for loop, e.g. `for(auto& port : ports)`




-------------------------------

Updated on 2022-01-24 at 14:10:27 -0500