# Serial::PortInfo


Constexpr range template to allow enumerator to be used as a range.  [More...](#detailed-description)


`#include <SerialEnumeration.hpp>`

## Public Functions

|                | Name           |
| -------------- | -------------- |
| [PortIter](struct_serial_1_1_port_iter.md) | **[begin](struct_serial_1_1_port_info.md#function-begin)**() const<br>Get iterator to begin enumeration.  |
| [PortIter](struct_serial_1_1_port_iter.md) | **[end](struct_serial_1_1_port_info.md#function-end)**() const<br>Get dummy (invalid) iterator for end of enumeration.  |

## Detailed Description

```cpp
template <BusType type>
struct Serial::PortInfo;
```

Constexpr range template to allow enumerator to be used as a range. 
## Public Functions Documentation

### function begin

```cpp
inline PortIter begin() const
```

Get iterator to begin enumeration. 

### function end

```cpp
inline PortIter end() const
```

Get dummy (invalid) iterator for end of enumeration. 

-------------------------------

Updated on 2022-01-24 at 14:35:49 -0500