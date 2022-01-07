#pragma once

#if defined(_WIN32)
#  if defined(EXPORTING_SERIAL)
#    define DECLSPEC __declspec(dllexport)
#  elif defined(SERIAL_STATIC)
#    define DECLSPEC
#  else
#    define DECLSPEC __declspec(dllimport)
#  endif
#else // non windows
#  define DECLSPEC [[gnu::visibility("default")]]
#endif

#ifndef WIN32
#include <termios.h>
#endif

namespace Serial
{
  // Define handle type here to avoid having to include Windows.h which pulls in a huge amount of junk
#ifdef WIN32
  typedef void* HANDLE;
#else
  typedef int HANDLE;
#endif

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

  enum class Parity
  {
    None  = 0,
    Even  = 2,
    Odd   = 1,
    Mark  = 3,
    Space = 4
  };

  enum class Stop
  {
    OneBit  = 0,
    TwoBits = 2
  };

  struct DECLSPEC Device
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
    /// \param binary      false: port is opened in text mode, true: port is opened in binary mode  
    /// \param dataBits    Number of data bits in a frame
    /// \param stop        Number of stop bits in a frame
    /// \param parity      Parity mode
    /// \param flowControl Should RTS/CTS flow control be used?
    /// \return 0 or system error code
    int configure(unsigned int baudRate, bool binary, unsigned char dataBits, Stop stop, Parity parity, bool flowControl) noexcept;

    /// \brief Set serial timeouts
    /// \param timeout Number of milliseconds to wait for IO before timing out
    /// \return 0 or system error code
    /// \remarks Setting the timeout to 0 will cause read operations to return immediately with the bytes currently available
    int setTimeout(unsigned int timeout) noexcept;

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

    ///  Get number of bytes in transmit buffer
    int transmitQueueLevel() noexcept;

    /// Get serial transmission errors encountered
    int errorFlags() const noexcept;

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
#endif
  };
}