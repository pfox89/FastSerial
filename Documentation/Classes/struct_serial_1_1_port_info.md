---
title: Serial::PortInfo
summary: Constexpr range template to allow enumerator to be used as a range. 

---

# Serial::PortInfo



Constexpr range template to allow enumerator to be used as a range.  [More...](#detailed-description)


`#include <SerialEnumeration.hpp>`

## Public Functions

|                | Name           |
| -------------- | -------------- |
| [PortIter](/Documentation/Classes/struct_serial_1_1_port_iter/) | **[begin](/Documentation/Classes/struct_serial_1_1_port_info/#function-begin)**() const<br>Get iterator to begin enumeration.  |
| [PortIter](/Documentation/Classes/struct_serial_1_1_port_iter/) | **[end](/Documentation/Classes/struct_serial_1_1_port_info/#function-end)**() const<br>Get dummy (invalid) iterator for end of enumeration.  |

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

Updated on 2022-01-24 at 14:10:27 -0500