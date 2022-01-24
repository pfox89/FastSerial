# Serial


## Namespaces

| Name           |
| -------------- |
| **[Serial::pci](namespace_serial_1_1pci.md)**  |
| **[Serial::platform](namespace_serial_1_1platform.md)**  |
| **[Serial::pnp](namespace_serial_1_1pnp.md)**  |
| **[Serial::usb](namespace_serial_1_1usb.md)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Serial::Device](struct_serial_1_1_device.md)**  |
| struct | **[Serial::DeviceInfo](struct_serial_1_1_device_info.md)** <br>Information about device.  |
| struct | **[Serial::Enum](struct_serial_1_1_enum.md)**  |
| struct | **[Serial::Frame](struct_serial_1_1_frame.md)** <br>Read buffer to accumulate reads so we can ensure we read a minimum amount of data.  |
| struct | **[Serial::FrameBuffer](struct_serial_1_1_frame_buffer.md)** <br>Encapsulates a buffer to receive a frame, with a certain maximum size. This class allows consecutive polling of the device until desired amount of data has been received.  |
| struct | **[Serial::PortInfo](struct_serial_1_1_port_info.md)** <br>Constexpr range template to allow enumerator to be used as a range.  |
| struct | **[Serial::PortIter](struct_serial_1_1_port_iter.md)** <br>Iterator to iterate over ports enumerator.  |

## Types

|                | Name           |
| -------------- | -------------- |
| enum class| **[BusType](namespace_serial.md#enum-bustype)** { BUS_UNKNOWN = 0, BUS_USB = 0x1, BUS_PCI = 0x2, BUS_PNP = 0x4, BUS_PLATFORM = 0x8, BUS_ANY = BUS_USB | BUS_PCI | BUS_PNP | BUS_PLATFORM}<br>Possible types of bus this device can be connected to.  |

## Functions

|                | Name           |
| -------------- | -------------- |
| const char * | **[to_cstring](namespace_serial.md#function-to-cstring)**(BusType type) |
| std::string | **[to_string](namespace_serial.md#function-to-string)**(BusType type) |
| std::ostream & | **[operator<<](namespace_serial.md#function-operator<<)**(std::ostream & os, BusType type) |
| std::ostream & | **[operator<<](namespace_serial.md#function-operator<<)**(std::ostream & os, const [DeviceInfo](struct_serial_1_1_device_info.md) & port)<br>Stream insertion operator to pretty-print serial device information.  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| constexpr [PortInfo](struct_serial_1_1_port_info.md)< BusType::BUS_ANY > | **[ports](namespace_serial.md#variable-ports)** <br>Enuemrator object for all ports Use begin() and end() functions to get iterators to port range, or use in range-based for loop, e.g. `for(auto& port : ports)` |

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

Updated on 2022-01-24 at 14:35:49 -0500