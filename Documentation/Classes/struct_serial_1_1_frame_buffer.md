---
title: Serial::FrameBuffer
summary: Encapsulates a buffer to receive a frame, with a certain maximum size. This class allows consecutive polling of the device until desired amount of data has been received. 

---

# Serial::FrameBuffer



Encapsulates a buffer to receive a frame, with a certain maximum size. This class allows consecutive polling of the device until desired amount of data has been received.  [More...](#detailed-description)


`#include <Serial.hpp>`

Inherits from [Serial::Frame](Classes/struct_serial_1_1_frame.md)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[FrameBuffer](Classes/struct_serial_1_1_frame_buffer.md#function-framebuffer)**(unsigned short desiredSize =MaxSize)<br>Create a frame.  |
| int | **[desired](Classes/struct_serial_1_1_frame_buffer.md#function-desired)**(unsigned short desired)<br>Change number of bytes to receive before considering frame complete.  |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| char | **[_dataBuffer](Classes/struct_serial_1_1_frame_buffer.md#variable--databuffer)** <br>Buffer to contain actual received data, starting with 2nd byte.  |

## Additional inherited members

**Public Functions inherited from [Serial::Frame](Classes/struct_serial_1_1_frame.md)**

|                | Name           |
| -------------- | -------------- |
| void | **[reset](Classes/struct_serial_1_1_frame.md#function-reset)**(unsigned short desired)<br>Reset buffer state for new read operation.  |

**Public Attributes inherited from [Serial::Frame](Classes/struct_serial_1_1_frame.md)**

|                | Name           |
| -------------- | -------------- |
| unsigned short | **[status](Classes/struct_serial_1_1_frame.md#variable-status)** <br>Number of bytes read so far.  |
| unsigned short | **[desired_size](Classes/struct_serial_1_1_frame.md#variable-desired-size)** <br>Total number of bytes to read, at minimum.  |
| unsigned short | **[wait_time](Classes/struct_serial_1_1_frame.md#variable-wait-time)** <br>Amount of time spent waiting where no bytes arrived.  |
| char | **[data](Classes/struct_serial_1_1_frame.md#variable-data)** <br>Flat data buffer (Size extended by subclass template)  |


## Detailed Description

```cpp
template <unsigned short MaxSize>
struct Serial::FrameBuffer;
```

Encapsulates a buffer to receive a frame, with a certain maximum size. This class allows consecutive polling of the device until desired amount of data has been received. 
## Public Functions Documentation

### function FrameBuffer

```cpp
inline FrameBuffer(
    unsigned short desiredSize =MaxSize
)
```

Create a frame. 

**Parameters**: 

  * **desiredSize** Minimum amount of data we want to read, if different from the maximum 


### function desired

```cpp
inline int desired(
    unsigned short desired
)
```

Change number of bytes to receive before considering frame complete. 

**Parameters**: 

  * **desired** New number of bytes to receive before considering frame complete 


**Return**: 0, or invalid_argument if desired number of bytes is too large for this buffer 

## Public Attributes Documentation

### variable _dataBuffer

```cpp
char _dataBuffer;
```

Buffer to contain actual received data, starting with 2nd byte. 

-------------------------------

Updated on 2022-01-24 at 13:50:34 -0500