#pragma once

#ifndef WIN32
#include <termios.h>
#endif

#include <system_error>

namespace Serial
{
  // Define handle type here to avoid having to include Windows.h which pulls in a huge amount of junk
#ifdef WIN32
  typedef void* HANDLE;
#else
  typedef int HANDLE;
#endif

  /// Describes which buffers should be purged
  enum class Purge
  {
#ifdef WIN32
    TX = 0x01 | 0x04, // PURGE_TXABORT | PURGE_TXCLEAR 
    RX = 0x02 | 0x08, // PURGE_RXABORT | PURGE_RXCLEAR
    All = TX | RX 
#else
    TX = TCOFLUSH,
    RX = TCIFLUSH,
    All = TCIOFLUSH
#endif
  };

  /// \brief Describes parity scheme to be used by serial device
  enum class Parity
  {
    None  = 0,
    Even  = 2,
    Odd   = 1
  };

  /// \brief Describes number of stop bits to be used for serial communication
  enum class Stop
  {
    OneBit  = 0,
    TwoBits = 2
  };

  /// \brief Bits for describing events that occurred
  enum class Event
  {
    Overflow    = 0x1,
    Overrun     = 0x2,
    ParityError = 0x4,
    FrameError  = 0x8,
    Break       = 0x10
  };

  struct Device
  {
    /// Default constructor
    Device() noexcept;

    /// Don't allow copying, we shouldn't have multiple instances trying to control a single device
    Device(const Device&) noexcept = delete;

    /// Move constructor
    Device(Device&& other) noexcept;

    /// \brief Open port at given path
    /// \param port Path to device to open
    /// \return 0 or system error code
    int open(const char* port) noexcept;

    /// \brief Configure open port settings
    /// \param baudRate    Baud rate in bits per second
    /// \param dataBits    Number of data bits in a frame
    /// \param stop        Number of stop bits in a frame
    /// \param parity      Parity mode
    /// \param flowControl Should RTS/CTS flow control be used?
    /// \return 0 or system error code
    int configure(unsigned int baudRate, unsigned char dataBits, Stop stop, Parity parity, bool flowControl, int timeout) noexcept;

    /// \brief Write bytes to serial port
    /// \param data  Pointer to data to write
    /// \param count Size of data to write
    /// \return Bytes written, or system error code if negative
    int write(const void* data, unsigned count) noexcept;

    /// \brief Read bytes from serial port
    /// \param data 
    /// \param count 
    /// \return Bytes read, or system error code if negative
    int read(void* data, unsigned count) noexcept;

    /// Get number of bytes in receive buffer
    int receiveQueueLevel() noexcept;

    /// Get number of bytes in transmit buffer
    int transmitQueueLevel() noexcept;

    /// Get Events that have occurred on this serial port since the last time this function was called
    /// \return Combination of Event flags, or system error code if negative
    int events() noexcept;

    /// \brief Wait until all transmitted data has been sent to device
    int flush() noexcept;

    /// \brief Purge serial port, clearing buffers of any unhandled data
    /// \param type Specify which buffers to purge
    int purge(Purge type) noexcept;

    /// \brief Close serial port
    int close() noexcept;

    ~Device() noexcept { close(); }

    /// brief Get native handle to serial port
    HANDLE native() noexcept { return _hPort; }

  private:
    HANDLE        _hPort;

#ifdef _WIN32
    unsigned long _events;
    unsigned long _errors;
#else
    int _overflowEvents;
    int _overrunEvents;
    int _breakEvents;
    int _frameEvents;
    int _parityEvents;
    int _timeout;
#endif
  };

  template<unsigned MaxSize>
  struct Frame
  {
    Frame(Device& dev, int desiredSize=MaxSize) noexcept
      : _readFrom(dev), _status(0), _desiredSize(desiredSize), _data()
    {}

    int read() noexcept
    {
      if(_status < _desiredSize)
      {
        int s = _readFrom.read(&_data[_status], _desiredSize-_status);
        if(s < 0) return s;
        else if(s == 0) return -static_cast<int>(std::errc::timed_out);
        else _status += s;
        
        if(_status < _desiredSize) return 0;
      }
      return _status;
    }

    Frame& desired(int desired) noexcept { _desiredSize = desired; return *this; }
    int  status() const noexcept { return _status; }
    int  desired() const noexcept { return _desiredSize; }
    const void* data() const noexcept { return _data; }
  private:
    Device& _readFrom;
    int  _status;
    int  _desiredSize;
    char _data[MaxSize];
  };
}