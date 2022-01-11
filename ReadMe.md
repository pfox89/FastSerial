# Fast Serial library
This library is intended to provide a cross platform library that implements the following:
- Serial port enumeration with friendly names and physical device paths.
- Precise timing of send and receive operations so that commands and responses to hardware can be timed down to system limits. This includes the ability to determine when writes have been flushed to the device.
- Sync and polled asynchronous I/O without using threads. This allows additional processing to happen on the same thread concurrently with serial transmit/receive for best performance/lowest latency.

## Testing
Automated testing can be performed with ctest by invoking the "test" target in the build directory, e.g. `ninja test` or by running `ctest` in the same folder

In order to successfully run loopback tests, a serial port configured to loop back output is required. The device path can be specified to cmake during the configure step with `-DLOOPBACK_SERIALPORT=<device_path>`.