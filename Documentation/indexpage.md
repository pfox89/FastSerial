---
title: Fast Serial library

---

# Fast Serial library



This library is intended to provide a cross platform library that implements the following:

* Serial port enumeration with friendly names and physical device paths.
* Precise timing of send and receive operations so that commands and responses to hardware can be timed down to system limits. This includes the ability to determine when writes have been flushed to the device.
* Sync and polled asynchronous I/O without using threads. This allows additional processing to happen on the same thread concurrently with serial transmit/receive for best performance/lowest latency.

# Serial Enumeration

Serial enumeration is performed by the [Serial::Enum](Classes/struct_serial_1_1_enum.md) class in [SerialEnumeration.hpp]. In general, the following procedure can be used:

* Call [Serial::Enum::begin()](Classes/struct_serial_1_1_enum.md#function-begin) to begin enumeration. Desired device types are specified as a bitmask
* Call [Serial::Enum::next()](Classes/struct_serial_1_1_enum.md#function-next) to get device information for the next enumerated device. If it returns > 0, copy any information in the DeviceInfo struct.
* When [Serial::Enum::next()](Classes/struct_serial_1_1_enum.md#function-next) returns 0, no more devices can be enumerated.
* Call [Serial::Enum::clear()](Classes/struct_serial_1_1_enum.md#function-clear) to free resources associated with the serial enumeration object when you are done.

# Serial Device

The [Serial::Device](Classes/struct_serial_1_1_device.md) class in [Serial.hpp](Files/_serial_8hpp.md#file-serial.hpp) implements a simple interface for a serial port. The goal here is to avoid complex, error-prone edge cases and focus on the most common, widely-supported modes.

To this end, there are 2 main modes:

* If timeout == 0, read() will return immediately with whatever bytes have been read without waiting.
* If timeout > 0, read() will wait until there is at least 1 byte in the buffer or the timeout has occurred, and then return with whatever bytes haave been read.


> Note that in both modes, there is no guarantee that the serial device will wait for all bytes requested to be read. Thus, it is usually necessary to buffer the read data and make multiple read calls until all expected data has been received, or the read has timed out. 
> 
> 

[Serial::FrameBuffer](Classes/struct_serial_1_1_frame_buffer.md) can help to buffer data until enough data has been received to process. If this is passed to Serial::Device::read(FrameBuffer&), it will incrementally read into the buffer and only return >0 if the complete frame has been received. Then, wait_time can be used to keep track of how long has been spent waiting for bytes to be received, which allows checking for a frame timeout.


# Testing

Automated testing can be performed with ctest by invoking the "test" target in the build directory, e.g. `ninja test` or by running `ctest` in the same folder

In order to successfully run loopback tests, a serial port configured to loop back output is required. The device path can be specified to cmake during the configure step with `-DLOOPBACK_SERIALPORT=<device_path>`. 

-------------------------------

Updated on 2022-01-24 at 13:50:34 -0500
