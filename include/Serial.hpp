#pragma once

/// \file Serial.hpp

#include <system_error>
#ifndef WIN32
#include <termios.h>
#endif

namespace Serial
{

/// \brief Read buffer to accumulate reads so we can ensure we read a minimum amount of data
struct Frame
{
  unsigned short status;       ///< Number of bytes read so far
  unsigned short desired_size; ///< Total number of bytes to read, at minimum
  unsigned short wait_time;    ///< Amount of time spent waiting where no bytes arrived
  char           data[1];      ///< Flat data buffer (Size extended by subclass template)

  /// \brief Reset buffer state for new read operation
  /// \param desired Minimum number of bytes to receive.
  void reset(unsigned short desired) noexcept
  {
    status       = 0;
    desired_size = desired;
    wait_time    = 0;
  }
};

/// This class implements a very simple serial interface that should be sufficient for most use-cases.
/// There are two primary modes:
/// - First-byte wait, with timeout > 0. In this mode, read() will wait until at least one byte arrives or until `timeout` ms have elapsed.
/// - Immediate, with timeout == 0. In this mode, read() will return immediately with whatever bytes have been read.
///
/// \note In either mode, read() may return fewer bytes than requested.
///       If you require a certain number of bytes, keep calling read() to get remaining bytes, unless it times out.
/// 
/// \note If you need to buffer data read until you have a certain amount of data, set the timeout to a reasonable value (1-100ms),
///       use the FrameBuffer class and call read(Frame&) until it returns > 0 or Frame::wait_time exceeds your desired timeout.
/// 
/// \note Conventional serial devices and drivers were designed for 1980s serial terminals and modems, and aren't high-performance or precise
///       In general, don't rely on timing accuracy better than 100ms.
///       If you need more accurate timing, a custom device with custom drivers is the best option.
struct Device
{

/// Definee platform-specific handle type to avoid pulling in platform headers
#ifdef WIN32
  typedef void* HANDLE;
#else
  typedef int HANDLE;
#endif

  /// Describes which buffers should be purged
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

  /// Describes parity scheme to be used by serial device
  enum class Parity
  {
    None = 0,
    Even = 2,
    Odd  = 1
  };

  /// Describes number of stop bits to be used for serial communication
  enum class Stop
  {
    OneBit  = 0,
    TwoBits = 2
  };

  /// Bits for describing events that occurred
  enum class Event
  {
    Overflow    = 0x1,
    Overrun     = 0x2,
    ParityError = 0x4,
    FrameError  = 0x8,
    Break       = 0x10
  };

  /// Default constructor
  Device() noexcept;

  /// Don't allow copying, we shouldn't have multiple instances trying to control a single device
  Device(const Device&) noexcept = delete;

  /// Move constructor
  Device(Device&& other) noexcept;

  /// \brief Open port at given path
  /// \param port Path to device to open
  /// \return == 0 Succeess
  ///          < 0 Error occurred, std::error_code(-ret, system_category()) for details.
  int open(const char* port) noexcept;

  /// \brief Configure serial port settings after opening
  /// \param baudRate    Baud rate in bits per second
  /// \param dataBits    Number of data bits in a frame
  /// \param stop        Number of stop bits in a frame
  /// \param parity      Parity mode
  /// \param flowControl Should RTS/CTS flow control be used?
  /// \param timeout     Time, in ms, to wait for first byte to arrive before returning available bytes
  /// \return == 0 Succeess
  ///          < 0 Error occurred, std::error_code(-ret, system_category()) for details.
  int configure(
    unsigned int   baudRate,
    unsigned char  dataBits,
    Stop           stop,
    Parity         parity,
    bool           flowControl,
    unsigned short timeout) noexcept;

  /// \brief Write bytes to serial port
  /// \param data  Pointer to data to write
  /// \param count Size of data to write
  /// \return Bytes written, or system error code if negative
  int write(const void* data, unsigned count) noexcept;

  /// \brief Read  bytes from serial port
  /// \param data  Pointer to destination data buffer
  /// \param count Number of bytes to read
  /// \return >= 0 Bytes read
  ///          < 0 Error occurred, std::error_code(-return, system_category()) for details.
  int read(void* data, unsigned count) noexcept;

  /// \brief Read frame from serial port, in polled fashion. This can be called repeatedly until frame is complete.
  /// \param f Frame with buffer and state of reads.
  /// \return == 0 Frame is not yet complete
  ///          > 0 Complete frame receieved, number of bytes
  ///          < 0 Error occurred, std::error_code(-return, system_category()) for details.
  /// \remarks This will add to the frame's wait_time if no bytes are received within the serial port's timeout interval.
  ///          This wait_time can be checked to see if 
  int read(Frame& f) noexcept;

  /// Get number of bytes in receive buffer
  int receiveQueueLevel() noexcept;

  /// Get number of bytes in transmit buffer
  int transmitQueueLevel() noexcept;

  /// Get Events that have occurred on this serial port since the last time this function was called
  /// \return Combination of Event flags, or system error code if negative
  int events() noexcept;

  /// Wait until all transmitted data has been sent to device
  /// \return == 0 Succeess
  ///          < 0 Error occurred, std::error_code(-return, system_category()) for details.
  int flush() noexcept;

  /// \brief Purge serial port, clearing buffers of any unhandled data
  /// \param type Specify which buffers to purge
  /// \return == 0 Succeess
  ///          < 0 Error occurred, std::error_code(-return, system_category()) for details.
  int purge(Purge type) noexcept;

  /// Close serial port
  int close() noexcept;

  ~Device() noexcept { close(); }

  /// Get native handle to serial port
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

/// \brief Encapsulates a buffer to receive a frame, with a certain maximum size.
/// This class allows consecutive polling of the device until desired amount of data has been received
template<unsigned short MaxSize>
struct FrameBuffer : Frame
{
  /// \brief Create a frame
  /// \param desiredSize Minimum amount of data we want to read, if different from the maximum
  FrameBuffer(unsigned short desiredSize = MaxSize) noexcept
    : Frame{ 0, desiredSize, 0 }
    , _dataBuffer{}
  {}

  /// \brief Change number of bytes to receive before considering frame complete
  /// \param desired New number of bytes to receive before considering frame complete
  /// \return 0, or invalid_argument if desired number of bytes is too large for this buffer
  int desired(unsigned short desired) noexcept 
  {
    if (desired > MaxSize) return std::errc::invalid_argument;
    desired_size = desired;
    return 0;
  }
  /// Buffer to contain actual received data, starting with 2nd byte
  char _dataBuffer[MaxSize - 1];
};

}