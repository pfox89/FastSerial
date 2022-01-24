---
title: include/Serial.hpp

---

# include/Serial.hpp



## Namespaces

| Name           |
| -------------- |
| **[Serial](/Documentation/Namespaces/namespace_serial/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Serial::Frame](/Documentation/Classes/struct_serial_1_1_frame/)** <br>Read buffer to accumulate reads so we can ensure we read a minimum amount of data.  |
| struct | **[Serial::Device](/Documentation/Classes/struct_serial_1_1_device/)**  |
| struct | **[Serial::FrameBuffer](/Documentation/Classes/struct_serial_1_1_frame_buffer/)** <br>Encapsulates a buffer to receive a frame, with a certain maximum size. This class allows consecutive polling of the device until desired amount of data has been received.  |




## Source code

```cpp
#pragma once


#include <system_error>
#ifndef WIN32
#include <termios.h>
#endif

namespace Serial
{

struct Frame
{
  unsigned short status;       
  unsigned short desired_size; 
  unsigned short wait_time;    
  char           data[1];      

  void reset(unsigned short desired) noexcept
  {
    status       = 0;
    desired_size = desired;
    wait_time    = 0;
  }
};

struct Device
{

#ifdef WIN32
  typedef void* HANDLE;
#else
  typedef int HANDLE;
#endif

  enum class Purge
  {
#ifdef WIN32
    TX  = 0x01 | 0x04, // PURGE_TXABORT | PURGE_TXCLEAR
    RX  = 0x02 | 0x08, // PURGE_RXABORT | PURGE_RXCLEAR
    All = TX | RX
#else
    TX  = TCOFLUSH,
    RX  = TCIFLUSH,
    All = TCIOFLUSH
#endif
  };

  enum class Parity
  {
    None = 0,
    Even = 2,
    Odd  = 1
  };

  enum class Stop
  {
    OneBit  = 0,
    TwoBits = 2
  };

  enum class Event
  {
    Overflow    = 0x1,
    Overrun     = 0x2,
    ParityError = 0x4,
    FrameError  = 0x8,
    Break       = 0x10
  };

  Device() noexcept;

  Device(const Device&) noexcept = delete;

  Device(Device&& other) noexcept;

  int open(const char* port) noexcept;

  int configure(
    unsigned int   baudRate,
    unsigned char  dataBits,
    Stop           stop,
    Parity         parity,
    bool           flowControl,
    unsigned short timeout) noexcept;

  int write(const void* data, unsigned count) noexcept;

  int read(void* data, unsigned count) noexcept;

  int read(Frame& f) noexcept;

  int receiveQueueLevel() noexcept;

  int transmitQueueLevel() noexcept;

  int events() noexcept;

  int flush() noexcept;

  int purge(Purge type) noexcept;

  int close() noexcept;

  ~Device() noexcept { close(); }

  HANDLE native() noexcept { return _hPort; }

private:
  HANDLE _hPort;

  unsigned short _timeout;
#ifdef _WIN32
  unsigned long _events;
  unsigned long _errors;
#else
  int _overflowEvents;
  int _overrunEvents;
  int _breakEvents;
  int _frameEvents;
  int _parityEvents;
#endif
};

template<unsigned short MaxSize>
struct FrameBuffer : Frame
{
  FrameBuffer(unsigned short desiredSize = MaxSize) noexcept
    : Frame{ 0, desiredSize, 0 }
    , _dataBuffer{}
  {}

  int desired(unsigned short desired) noexcept 
  {
    if (desired > MaxSize) return std::errc::invalid_argument;
    desired_size = desired;
    return 0;
  }
  char _dataBuffer[MaxSize - 1];
};

}
```


-------------------------------

Updated on 2022-01-24 at 14:10:27 -0500
