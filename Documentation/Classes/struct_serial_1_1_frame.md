---
title: Serial::Frame
summary: Read buffer to accumulate reads so we can ensure we read a minimum amount of data. 

---

# Serial::Frame



Read buffer to accumulate reads so we can ensure we read a minimum amount of data. 


`#include <Serial.hpp>`

Inherited by [Serial::FrameBuffer< MaxSize >](/Documentation/Classes/struct_serial_1_1_frame_buffer/)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| void | **[reset](/Documentation/Classes/struct_serial_1_1_frame/#function-reset)**(unsigned short desired)<br>Reset buffer state for new read operation.  |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| unsigned short | **[status](/Documentation/Classes/struct_serial_1_1_frame/#variable-status)** <br>Number of bytes read so far.  |
| unsigned short | **[desired_size](/Documentation/Classes/struct_serial_1_1_frame/#variable-desired-size)** <br>Total number of bytes to read, at minimum.  |
| unsigned short | **[wait_time](/Documentation/Classes/struct_serial_1_1_frame/#variable-wait-time)** <br>Amount of time spent waiting where no bytes arrived.  |
| char | **[data](/Documentation/Classes/struct_serial_1_1_frame/#variable-data)** <br>Flat data buffer (Size extended by subclass template)  |

## Public Functions Documentation

### function reset

```cpp
inline void reset(
    unsigned short desired
)
```

Reset buffer state for new read operation. 

**Parameters**: 

  * **desired** Minimum number of bytes to receive. 


## Public Attributes Documentation

### variable status

```cpp
unsigned short status;
```

Number of bytes read so far. 

### variable desired_size

```cpp
unsigned short desired_size;
```

Total number of bytes to read, at minimum. 

### variable wait_time

```cpp
unsigned short wait_time;
```

Amount of time spent waiting where no bytes arrived. 

### variable data

```cpp
char data;
```

Flat data buffer (Size extended by subclass template) 

-------------------------------

Updated on 2022-01-24 at 14:10:27 -0500