#include "Serial.hpp"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#include <Windows.h>

static constexpr DWORD event_mask = EV_RXCHAR | EV_TXEMPTY | EV_ERR | EV_BREAK;


namespace Serial
{
  Device::Device() noexcept
    : _hPort(INVALID_HANDLE_VALUE), _errors(), _events()
  {}

  Device::Device(Device&& other) noexcept
    : _hPort(other._hPort),_errors(other._errors), _events(other._events)
  {
    other._hPort = INVALID_HANDLE_VALUE;
  }

  int Device::open(const char* port) noexcept
  {
    _hPort = CreateFileA(port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, 0);

    if (INVALID_HANDLE_VALUE == _hPort) return -static_cast<int>(GetLastError());

    if (false == SetCommMask(_hPort, event_mask)) return -static_cast<int>(GetLastError());

    return 0;
  }

  int Device::configure(unsigned int baudRate, bool binary, unsigned char dataBits, Stop stop, Parity parity, bool flowControl) noexcept
  {
    DCB dcb{
        sizeof(DCB), // DCBLength
        baudRate,    // BaudRate
        binary,      // fBinary
        parity == Parity::None ? 0U : 1U, // fParity
        flowControl, // fOutxCtsFlow
        0, // fOutxDsrFlow
        0, // fDtrControl
        0, // fDsrSensitivity
        0, // fTXContinueOnXoff
        0, // fOutX
        0, // fInX
        0, // fErrorChar
        0, // fNull
        static_cast<DWORD>(flowControl ? RTS_CONTROL_TOGGLE : RTS_CONTROL_DISABLE), // fRtsControl
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
    if (false == SetCommState(_hPort, &dcb)) return -static_cast<int>(GetLastError());
    return 0;
  }

  int Device::setTimeout(unsigned int timeout) noexcept
  {
    COMMTIMEOUTS co;
    if (timeout > 0)
    {
      // Wait timeout ms or until at least 1 byte is in receive buffer and return with bytes in receive buffer
      // Similar to POSIX non-canonoical mode with VMIN=0
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
      co.WriteTotalTimeoutConstant = 50; // If timeout == 0, we want writes to timeout if they can't be completed in a reasonable amount of time (analogous to NDELAY on POSIX)
      co.WriteTotalTimeoutMultiplier = 0;
    }

    if (false == SetCommTimeouts(_hPort, &co)) return -static_cast<int>(GetLastError());
    return 0;
  }


  int Device::write(const void* data, unsigned count) noexcept
  {
    DWORD written;

    if (false == WriteFile(_hPort, data, count, &written, nullptr))
    {
      return -static_cast<int>(GetLastError());
    }
    else
    {
      return written;
    }
  }

  int Device::read(void* data, unsigned count) noexcept
  {
    DWORD read;
    int total;
    if (false == ReadFile(_hPort, data, count, &read, nullptr))
    {
      return -static_cast<int>(GetLastError());
    }
    if (read < count)
    {
      total = read;
      if (false == ReadFile(_hPort, static_cast<char*>(data) + total, count, &read, nullptr))
      {
        return -static_cast<int>(GetLastError());
      }
      else
        read += total;
    }
    return read;
  }

  int  Device::receiveQueueLevel() noexcept
  {
    COMSTAT status;
    if (false == ClearCommError(_hPort, &_errors, &status)) return -static_cast<int>(GetLastError());
    return status.cbInQue;
  }

  int Device::transmitQueueLevel() noexcept
  {
    COMSTAT status;
    if (false == ClearCommError(_hPort, &_errors, &status)) return -static_cast<int>(GetLastError());
    return status.cbOutQue;
  }

  /*
  int Device::poll(unsigned int timeout) noexcept
  {
    if (false == ClearCommError(_hPort, &_errors, &_status)) return GetLastError();

    if (_status.cbInQue == 0 && timeout > 0)
    {
      if (WaitCommEvent(_hPort, &_events, &_eventOverlapped))
      {
        return _events;
      }
      else
      {
        DWORD error = GetLastError();
        if (error == ERROR_IO_PENDING)
        {
          DWORD dummy;
          if (GetOverlappedResultEx(_hPort, &_eventOverlapped, &dummy, timeout, false))
          {
            return _events;
          }
          else
          {
            // calling SetCommMask forces WaitCommEvent() to cancel
            SetCommMask(_hPort, event_mask);
            // Update status to ensure we didn't miss anything between the wait and cancellation
            ClearCommError(_hPort, &_errors, &_status);
          }
        }
        else
        {
          return error;
        }
      }
    }

    if (_status.cbInQue != 0)
    {
      _events |= EV_RXCHAR;
    }
    if (_status.cbOutQue == 0)
    {
      _events |= EV_TXEMPTY;
    }

    return _events;
  }
  */

  int Device::errorFlags() const noexcept
  {
    return _errors;
  }

  int Device::purge(Purge type) noexcept
  {
    if (false == PurgeComm(_hPort, static_cast<DWORD>(type))) return -static_cast<int>(GetLastError());
    return 0;
  }

  int Device::close() noexcept
  {
    int error = 0;
    if (_hPort != INVALID_HANDLE_VALUE)
    {
      if (false == CloseHandle(_hPort)) error = -static_cast<int>(GetLastError());
      _hPort = INVALID_HANDLE_VALUE;
    }
    return error;
  }
}
