#pragma once

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#include <Windows.h>

namespace SerialXP
{

  enum class Purge
  {
    TX = PURGE_TXABORT,
    RX = PURGE_RXABORT,
    TXRX = PURGE_TXABORT | PURGE_RXABORT
  };

  enum class Parity
  {
    None = NOPARITY,
    Even = EVENPARITY,
    Odd  = ODDPARITY,
    Mark = MARKPARITY,
    Space = SPACEPARITY
  };

  enum class Stop
  {
    OneBit = ONESTOPBIT,
    OnePointFive = ONE5STOPBITS,
    TwoBits = TWOSTOPBITS
  };

  class Device
  {
    static constexpr DWORD event_mask = EV_RXCHAR | EV_TXEMPTY | EV_ERR | EV_BREAK;
  public:
    constexpr Device() noexcept = default;

    Device(const Device&) noexcept = delete;

    Device(Device&& other) noexcept
      : _hPort(other._hPort), _eventOverlapped(other._eventOverlapped)
    {
      other._hPort = INVALID_HANDLE_VALUE;
      other._eventOverlapped.hEvent = INVALID_HANDLE_VALUE;
    }

    int open(const char* port) noexcept
    {
      _hPort = CreateFileA(port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH | FILE_FLAG_OVERLAPPED, 0);
 
      if (INVALID_HANDLE_VALUE == _hPort) return GetLastError();
      _eventOverlapped = { 0 };

      _eventOverlapped.hEvent = CreateEventA(
        NULL,   // default security attributes 
        TRUE,   // manual-reset event 
        TRUE,   // start signaled, overlapped operations will set to not signaled
        NULL    // no name
      );
      if (INVALID_HANDLE_VALUE == _eventOverlapped.hEvent) return GetLastError();
      

      SetCommMask(_Port, event_mask);

      else return 0;
    }


    int configure(unsigned int baudRate, bool binary, unsigned char dataBits, Stop stop, Parity parity) noexcept
    {
      DCB dcb{
          sizeof(DCB), // DCBLength
          baudRate,    // BaudRate
          binary,      // fBinary
          parity == Parity::None ? 0 : 1, // fParity
          0, // fOutxCtsFlow
          0, // fOutxDsrFlow
          0, // fDtrControl
          0, // fDsrSensitivity
          0, // fTXContinueOnXoff
          0, // fOutX
          0, // fInX
          0, // fErrorChar
          0, // fNull
          0, // fRtsControl
          0, // fAbortOnError
          0, // fDummy2
          0, // wReserved
          0, // XonLim
          0, // XoffLim
          static_cast<BYTE>(dataBits), // ByteSize
          static_cast<BYTE>(parity), // Parity
          static_cast<BYTE>(stop), // StopBits
          0, // XonChar
          0, // XoffChar
          0, // ErrorChar
          0, // EofChar
          0, // EvtChar
          0, // wReserved1
      };
      if (false == SetCommState(_hPort)) return GetLastError();
      return 0;
    }


    int setTimeout(unsigned int timeout) noexcept
    {
      COMMTIMEOUTS co;
      if (timeout > 0)
      {
          // This should implement something similar POSIX non-canonical mode with VMIN=0
          co.ReadIntervalTimeout = MAXDWORD;
          co.ReadTotalTimeoutMultiplier = MAXDWORD;
          co.ReadTotalTimeoutConstant = timeout;
          co.WriteTotalTimeoutConstant = 0;
          co.WriteTotalTimeoutMultiplier = 0;
      }
      else
      {
        // A value of MAXDWORD, combined with zero values for both 
        // the ReadTotalTimeoutConstant and ReadTotalTimeoutMultiplier members,
        // specifies that the read operation is to return immediately 
        // with the bytes that have already been received, 
        // even if no bytes have been received.
        co.ReadIntervalTimeout = MAXDWORD;
        co.ReadTotalTimeoutConstant = 0;
        co.ReadTotalTimeoutMultiplier = 0;
        co.WriteTotalTimeoutConstant = 0;
        co.WriteTotalTimeoutMultiplier = 0;
      }

      if (false == SetCommTimeouts(_hPort, &co)) return GetLastError();
      return 0;
    }

    int write(const void* data, size_t count) noexcept
    {
      DWORD written;
      
      if (false == WriteFile(_hPort, data, count, &written, nullptr))
      {
        return GetLastError();
      }
      else
      {
        return written;
      }
    }

    int read(void* data, size_t count) noexcept
    {
      DWORD read;
      if (false == ReadFile(_hPort, data, count, &read, nullptr))
      {
        return GetLastError();
      }
      else
      {
        return read;
      }
    }

    int receiveQueueLength() const noexcept
    {
      return _status.cbInQue;
    }

    int transmitQueueLength() const noexcept
    {
      return _status.cbOutQue;
    }

    int errorFlags() const noexcept
    {
      return _errors;
    }

    int poll(unsigned int timeout) noexcept
    {
      ClearCommError(_hPort, &_errors, &_status);
      if (_status.cbInQue == 0)
      {
        if (WaitCommEvent(_hPort, &_events, _eventOverlapped))
        {
          return _events;
        }
        else
        {
          DWORD error = GetLastError();
          if (error == ERROR_IO_PENDING)
          {
            error = WaitForSingleObject(_eventOverlapped.hEvent, timeout);
            DWORD dummy;
            switch (error)
            {
              case WAIT_OBJECT_0;
                if (GetOverlappedResult(_hPort, _eventOverlapped, &dummy, false))
                  return _events;
                else
                  return GetLastError();
              case WAIT_TIMEOUT:
                // calling SetCommMask forces WaitCommEvent() to cancel
                SetCommMask(_hPort, event_mask);
                return 0;
              case WAIT_FAILED:
                return GetLastError();
              case WAIT_ABANDONED:
              default:
                // Something went wrong, but there's no way to get more details here
                return 0;
            }
          }
          else
          {
            return error;
          }
        }
      }
      return _events;
    }


    int purge(Purge type) noexcept
    {
      if (false == PurgeComm(_hPort, type)) return GetLastError();
      return 0;
    }

    int close() noexcept
    {
      int error = 0;
      if (_hPort != INVALID_HANDLE_VALUE)
      {
        if (!CloseHandle(_hPort)) error = GetLastError();
        _hPort = INVALID_HANDLE_VALUE;
      }
      if (other._eventOverlapped.hEvent != INVALID_HANDLE_VALUE)
      {
        if (!CloseHandle(other._eventOverlapped.hEvent)) error = GetLastError();
        other._eventOverlapped.hEvent = INVALID_HANDLE_VALUE;
      }
      return error;
    }

    ~Device() noexcept
    {
      close();
    }
  private:
    HANDLE _hPort;
    DWORD  _events;
    DWORD  _errors;
    OVERLAPPED _eventOverlapped;
    COMSTAT _status;
  };
}