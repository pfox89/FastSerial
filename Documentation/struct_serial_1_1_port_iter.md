# Serial::PortIter


Iterator to iterate over ports enumerator. 


`#include <SerialEnumeration.hpp>`

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[PortIter](struct_serial_1_1_port_iter/#function-portiter)**(unsigned type_mask) |
| | **[PortIter](struct_serial_1_1_port_iter/#function-portiter)**(int )<br>Passing an int constructs an dummy "end" iterator by setting status to invalid value.  |
| | **[PortIter](struct_serial_1_1_port_iter/#function-portiter)**(const [PortIter](struct_serial_1_1_port_iter/) & ) =delete<br>Don't allow copying iterator, ownership must be retained.  |
| | **[PortIter](struct_serial_1_1_port_iter/#function-portiter)**([PortIter](struct_serial_1_1_port_iter/) && other)<br>Move iterator takes ownership.  |
| [PortIter](struct_serial_1_1_port_iter/) & | **[operator++](struct_serial_1_1_port_iter/#function-operator++)**()<br>Incrementing iterator gets next device.  |
| const SerialDeviceInfo & | **[operator*](struct_serial_1_1_port_iter/#function-operator*)**() const<br>Get reference to device info for current enumerated device (exceptions disabled, no check for validity)  |
| const SerialDeviceInfo * | **[operator->](struct_serial_1_1_port_iter/#function-operator->)**() const<br>Get pointer to device info for current enumerated device (exceptions disabled, no check for validity)  |
| bool | **[operator!=](struct_serial_1_1_port_iter/#function-operator!=)**(const [PortIter](struct_serial_1_1_port_iter/) & ) const<br>Comparison operator does not inspect other iterator, only current status to determine the enumeration has reached the end.  |
| std::error_code | **[error](struct_serial_1_1_port_iter/#function-error)**() const<br>Get error that occurred while enumerating, if any.  |

## Public Functions Documentation

### function PortIter

```cpp
inline PortIter(
    unsigned type_mask
)
```


**Parameters**: 

  * **type_mask** Bitwise combination of any BusType values defining what kind of devices to include 


Create iterator over ports using type mask 


### function PortIter

```cpp
inline PortIter(
    int 
)
```

Passing an int constructs an dummy "end" iterator by setting status to invalid value. 

### function PortIter

```cpp
PortIter(
    const PortIter & 
) =delete
```

Don't allow copying iterator, ownership must be retained. 

### function PortIter

```cpp
inline PortIter(
    PortIter && other
)
```

Move iterator takes ownership. 

### function operator++

```cpp
inline PortIter & operator++()
```

Incrementing iterator gets next device. 

### function operator*

```cpp
inline const SerialDeviceInfo & operator*() const
```

Get reference to device info for current enumerated device (exceptions disabled, no check for validity) 

### function operator->

```cpp
inline const SerialDeviceInfo * operator->() const
```

Get pointer to device info for current enumerated device (exceptions disabled, no check for validity) 

### function operator!=

```cpp
inline bool operator!=(
    const PortIter & 
) const
```

Comparison operator does not inspect other iterator, only current status to determine the enumeration has reached the end. 

### function error

```cpp
inline std::error_code error() const
```

Get error that occurred while enumerating, if any. 

-------------------------------

Updated on 2022-01-24 at 14:34:17 -0500