# Fast Serial library
This library is intended to provide a cross platform library that implements the following:
- Serial port enumeration with friendly names and physical device paths.
- Precise timing of send and receive operations so that commands and responses to hardware can be timed down to system limits. This includes the ability to determine when writes have been flushed to the device.
- Sync and polled asynchronous I/O without using threads. This allows additional processing to happen on the same thread concurrently with serial transmit/receive for best performance/lowest latency.