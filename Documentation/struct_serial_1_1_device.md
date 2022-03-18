# Serial::Device


 [More...](#detailed-description)


`#include <Serial.hpp>`

## Public Types

|                | Name           |
| -------------- | -------------- |
| enum class| **[Purge](struct_serial_1_1_device.md#enum-purge)** { TX = TCOFLUSH, RX = TCIFLUSH, All = TCIOFLUSH}<br>Describes which buffers should be purged.  |
| enum class| **[Parity](struct_serial_1_1_device.md#enum-parity)** { None = 0, Even = 2, Odd = 1}<br>Describes parity scheme to be used by serial device.  |
| enum class| **[Stop](struct_serial_1_1_device.md#enum-stop)** { OneBit = 0, TwoBits = 2}<br>Describes number of stop bits to be used for serial communication.  |
| enum class| **[Event](struct_serial_1_1_device.md#enum-event)** { Overflow = 0x1, Overrun = 0x2, ParityError = 0x4, FrameError = 0x8, Break = 0x10}<br>Bits for describing events that occurred.  |
| typedef int | **[HANDLE](struct_serial_1_1_device.md#typedef-handle)** <br>Definee platform-specific handle type to avoid pulling in platform headers.  |
| typedef timespec | **[TIMESTAMP](struct_serial_1_1_device.md#typedef-timestamp)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[Device](struct_serial_1_1_device.md#function-device)**()<br>Default constructor.  |
| | **[Device](struct_serial_1_1_device.md#function-device)**(const [Device](struct_serial_1_1_device.md) & ) =delete<br>Don't allow copying, we shouldn't have multiple instances trying to control a single device.  |
| | **[Device](struct_serial_1_1_device.md#function-device)**([Device](struct_serial_1_1_device.md) && other)<br>Move constructor.  |
| int | **[open](struct_serial_1_1_device.md#function-open)**(const char * port)<br>Open port at given path.  |
| int | **[configure](struct_serial_1_1_device.md#function-configure)**(unsigned int baudRate, unsigned char dataBits, [Stop](struct_serial_1_1_device.md#enum-stop) stop, [Parity](struct_serial_1_1_device.md#enum-parity) parity, bool flowControl, unsigned short timeout)<br>Configure serial port settings after opening.  |
| int | **[write](struct_serial_1_1_device.md#function-write)**(const void * data, unsigned count)<br>Write bytes to serial port.  |
| int | **[read](struct_serial_1_1_device.md#function-read)**(void * data, unsigned count)<br>Read bytes from serial port.  |
| int | **[read](struct_serial_1_1_device.md#function-read)**([Frame](struct_serial_1_1_frame.md) & f)<br>Read frame from serial port, in polled fashion. This can be called repeatedly until frame is complete.  |
| int | **[receiveQueueLevel](struct_serial_1_1_device.md#function-receivequeuelevel)**()<br>Get number of bytes in receive buffer.  |
| int | **[transmitQueueLevel](struct_serial_1_1_device.md#function-transmitqueuelevel)**()<br>Get number of bytes in transmit buffer.  |
| int | **[events](struct_serial_1_1_device.md#function-events)**() |
| int | **[flush](struct_serial_1_1_device.md#function-flush)**() |
| int | **[purge](struct_serial_1_1_device.md#function-purge)**([Purge](struct_serial_1_1_device.md#enum-purge) type)<br>Purge serial port, clearing buffers of any unhandled data.  |
| int | **[close](struct_serial_1_1_device.md#function-close)**()<br>Close serial port.  |
| | **[~Device](struct_serial_1_1_device.md#function-~device)**() |
| [HANDLE](struct_serial_1_1_device.md#typedef-handle) | **[native](struct_serial_1_1_device.md#function-native)**()<br>Get native handle to serial port.  |
| unsigned long | **[timestamp](struct_serial_1_1_device.md#function-timestamp)**() const |

## Detailed Description

```cpp
struct Serial::Device;
```


**Note**: 

  * In either mode, [read()](struct_serial_1_1_device.md#function-read) may return fewer bytes than requested. If you require a certain number of bytes, keep calling [read()](struct_serial_1_1_device.md#function-read) to get remaining bytes, unless it times out.
  * If you need to buffer data read until you have a certain amount of data, set the timeout to a reasonable value (1-100ms), use the [FrameBuffer](struct_serial_1_1_frame_buffer.md) class and call [read(Frame&)](struct_serial_1_1_device.md#function-read) until it returns > 0 or [Frame::wait_time](struct_serial_1_1_frame.md#variable-wait-time) exceeds your desired timeout.
  * Conventional serial devices and drivers were designed for 1980s serial terminals and modems, and aren't high-performance or precise In general, don't rely on timing accuracy better than 100ms. If you need more accurate timing, a custom device with custom drivers is the best option. 


This class implements a very simple serial interface that should be sufficient for most use-cases. There are two primary modes:

* First-byte wait, with timeout > 0. In this mode, [read()](struct_serial_1_1_device.md#function-read) will wait until at least one byte arrives or until `timeout` ms have elapsed.
* Immediate, with timeout == 0. In this mode, [read()](struct_serial_1_1_device.md#function-read) will return immediately with whatever bytes have been read.

## Public Types Documentation

### enum Purge

| Enumerator | Value | Description |
| ---------- | ----- | ----------- |
| TX | TCOFLUSH|   |
| RX | TCIFLUSH|   |
| All | TCIOFLUSH|   |



Describes which buffers should be purged. 

### enum Parity

| Enumerator | Value | Description |
| ---------- | ----- | ----------- |
| None | 0|   |
| Even | 2|   |
| Odd | 1|   |



Describes parity scheme to be used by serial device. 

### enum Stop

| Enumerator | Value | Description |
| ---------- | ----- | ----------- |
| OneBit | 0|   |
| TwoBits | 2|   |



Describes number of stop bits to be used for serial communication. 

### enum Event

| Enumerator | Value | Description |
| ---------- | ----- | ----------- |
| Overflow | 0x1|   |
| Overrun | 0x2|   |
| ParityError | 0x4|   |
| FrameError | 0x8|   |
| Break | 0x10|   |



Bits for describing events that occurred. 

### typedef HANDLE

```cpp
typedef int Serial::Device::HANDLE;
```

Definee platform-specific handle type to avoid pulling in platform headers. 

### typedef TIMESTAMP

```cpp
typedef timespec Serial::Device::TIMESTAMP;
```


## Public Functions Documentation

### function Device

```cpp
Device()
```

Default constructor. 

### function Device

```cpp
Device(
    const Device & 
) =delete
```

Don't allow copying, we shouldn't have multiple instances trying to control a single device. 

### function Device

```cpp
Device(
    Device && other
)
```

Move constructor. 

### function open

```cpp
int open(
    const char * port
)
```

Open port at given path. 

**Parameters**: 

  * **port** Path to device to open 


**Return**: == 0 Succeess < 0 Error occurred, std::error_code(-ret, system_category()) for details. 

### function configure

```cpp
int configure(
    unsigned int baudRate,
    unsigned char dataBits,
    Stop stop,
    Parity parity,
    bool flowControl,
    unsigned short timeout
)
```

Configure serial port settings after opening. 

**Parameters**: 

  * **baudRate** Baud rate in bits per second 
  * **dataBits** Number of data bits in a frame 
  * **stop** Number of stop bits in a frame 
  * **parity** Parity mode 
  * **flowControl** Should RTS/CTS flow control be used? 
  * **timeout** Time, in ms, to wait for first byte to arrive before returning available bytes 


**Return**: == 0 Succeess < 0 Error occurred, std::error_code(-ret, system_category()) for details. 

### function write

```cpp
int write(
    const void * data,
    unsigned count
)
```

Write bytes to serial port. 

**Parameters**: 

  * **data** Pointer to data to write 
  * **count** Size of data to write 


**Return**: Bytes written, or system error code if negative 

### function read

```cpp
int read(
    void * data,
    unsigned count
)
```

Read bytes from serial port. 

**Parameters**: 

  * **data** Pointer to destination data buffer 
  * **count** Number of bytes to read 


**Return**: >= 0 Bytes read < 0 Error occurred, std::error_code(-return, system_category()) for details. 

### function read

```cpp
int read(
    Frame & f
)
```

Read frame from serial port, in polled fashion. This can be called repeatedly until frame is complete. 

**Parameters**: 

  * **f** [Frame](struct_serial_1_1_frame.md) with buffer and state of reads. 


**Return**: == 0 [Frame](struct_serial_1_1_frame.md) is not yet complete > 0 Complete frame receieved, number of bytes < 0 Error occurred, std::error_code(-return, system_category()) for details. 

**Remark**: This will add to the frame's wait_time if no bytes are received within the serial port's timeout interval. This wait_time can be checked to see if 

### function receiveQueueLevel

```cpp
int receiveQueueLevel()
```

Get number of bytes in receive buffer. 

### function transmitQueueLevel

```cpp
int transmitQueueLevel()
```

Get number of bytes in transmit buffer. 

### function events

```cpp
int events()
```


**Return**: Combination of Event flags, or system error code if negative 

Get Events that have occurred on this serial port since the last time this function was called 


### function flush

```cpp
int flush()
```


**Return**: == 0 Succeess < 0 Error occurred, std::error_code(-return, system_category()) for details. 

Wait until all transmitted data has been sent to device 


### function purge

```cpp
int purge(
    Purge type
)
```

Purge serial port, clearing buffers of any unhandled data. 

**Parameters**: 

  * **type** Specify which buffers to purge 


**Return**: == 0 Succeess < 0 Error occurred, std::error_code(-return, system_category()) for details. 

### function close

```cpp
int close()
```

Close serial port. 

### function ~Device

```cpp
inline ~Device()
```


### function native

```cpp
inline HANDLE native()
```

Get native handle to serial port. 

### function timestamp

```cpp
unsigned long timestamp() const
```


-------------------------------

Updated on 2022-03-18 at 12:53:42 -0400