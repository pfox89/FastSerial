# Serial::DeviceInfo


Information about device. 


`#include <SerialEnumeration.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| const char * | **[id](struct_serial_1_1_device_info/#variable-id)** <br>Logical device path (e.g. interface)  |
| const char * | **[path](struct_serial_1_1_device_info/#variable-path)** <br>Physical device path.  |
| const char * | **[name](struct_serial_1_1_device_info/#variable-name)** <br>[Device](struct_serial_1_1_device/) name.  |
| const char * | **[description](struct_serial_1_1_device_info/#variable-description)** <br>[Device](struct_serial_1_1_device/) description.  |
| const char * | **[manufacturer](struct_serial_1_1_device_info/#variable-manufacturer)** <br>[Device](struct_serial_1_1_device/) manufacturer string.  |
| unsigned short | **[vid](struct_serial_1_1_device_info/#variable-vid)** <br>Numeric vendor id (VID on USB, VEN on PCI)  |
| unsigned short | **[pid](struct_serial_1_1_device_info/#variable-pid)** <br>Numeric device id (PID on USB, DEV on PCI)  |
| BusType | **[type](struct_serial_1_1_device_info/#variable-type)** <br>Type of bus device is connected to.  |

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

[Device](struct_serial_1_1_device/) name. 

### variable description

```cpp
const char * description;
```

[Device](struct_serial_1_1_device/) description. 

### variable manufacturer

```cpp
const char * manufacturer;
```

[Device](struct_serial_1_1_device/) manufacturer string. 

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

Updated on 2022-01-24 at 14:34:17 -0500