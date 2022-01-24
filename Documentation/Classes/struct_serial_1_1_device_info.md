---
title: Serial::DeviceInfo
summary: Information about device. 

---

# Serial::DeviceInfo



Information about device. 


`#include <SerialEnumeration.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| const char * | **[id](Classes/struct_serial_1_1_device_info.md#variable-id)** <br>Logical device path (e.g. interface)  |
| const char * | **[path](Classes/struct_serial_1_1_device_info.md#variable-path)** <br>Physical device path.  |
| const char * | **[name](Classes/struct_serial_1_1_device_info.md#variable-name)** <br>[Device](Classes/struct_serial_1_1_device.md) name.  |
| const char * | **[description](Classes/struct_serial_1_1_device_info.md#variable-description)** <br>[Device](Classes/struct_serial_1_1_device.md) description.  |
| const char * | **[manufacturer](Classes/struct_serial_1_1_device_info.md#variable-manufacturer)** <br>[Device](Classes/struct_serial_1_1_device.md) manufacturer string.  |
| unsigned short | **[vid](Classes/struct_serial_1_1_device_info.md#variable-vid)** <br>Numeric vendor id (VID on USB, VEN on PCI)  |
| unsigned short | **[pid](Classes/struct_serial_1_1_device_info.md#variable-pid)** <br>Numeric device id (PID on USB, DEV on PCI)  |
| BusType | **[type](Classes/struct_serial_1_1_device_info.md#variable-type)** <br>Type of bus device is connected to.  |

## Public Attributes Documentation

### variable id

```cpp
const char * id;
```

Logical device path (e.g. interface) 

### variable path

```cpp
const char * path;
```

Physical device path. 

### variable name

```cpp
const char * name;
```

[Device](Classes/struct_serial_1_1_device.md) name. 

### variable description

```cpp
const char * description;
```

[Device](Classes/struct_serial_1_1_device.md) description. 

### variable manufacturer

```cpp
const char * manufacturer;
```

[Device](Classes/struct_serial_1_1_device.md) manufacturer string. 

### variable vid

```cpp
unsigned short vid;
```

Numeric vendor id (VID on USB, VEN on PCI) 

### variable pid

```cpp
unsigned short pid;
```

Numeric device id (PID on USB, DEV on PCI) 

### variable type

```cpp
BusType type;
```

Type of bus device is connected to. 

-------------------------------

Updated on 2022-01-24 at 13:50:34 -0500