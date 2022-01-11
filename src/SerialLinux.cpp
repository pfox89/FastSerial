#include "Serial.hpp"

#include <cerrno>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>

#include <linux/serial.h>

#define INVALID_HANDLE_VALUE -1

namespace Serial
{
  Device::Device() noexcept
    : _hPort(INVALID_HANDLE_VALUE),  _overflowEvents(), _overrunEvents(), _breakEvents(), _frameEvents(), _parityEvents()
  {}

  Device::Device(Device&& other) noexcept
    : _hPort(other._hPort), 
      _overflowEvents(other._overflowEvents), _overrunEvents(other._overrunEvents), 
      _breakEvents(other._breakEvents), _frameEvents(other._frameEvents), _parityEvents(other._parityEvents)
  {
    other._hPort = INVALID_HANDLE_VALUE;
  }

  int Device::open(const char* port) noexcept
  {
    _hPort = ::open(port, O_RDWR | O_NOCTTY);

    if (INVALID_HANDLE_VALUE == _hPort) return -errno;
    return 0;
  }

  int Device::configure(unsigned int baudRate, unsigned char dataBits, Stop stop, Parity parity, bool flowControl, int timeout) noexcept
  {
    termios options;
    if(0 != tcgetattr(_hPort, &options))
      return -errno;

    int baud;
    #define BAUDDEF(RATE) case RATE: baud = B##RATE; break
    switch(baudRate)
    {
      BAUDDEF(50);
      BAUDDEF(75);
      BAUDDEF(110);
      BAUDDEF(134);
      BAUDDEF(150);
      BAUDDEF(200);
      BAUDDEF(300);
      BAUDDEF(600);
      BAUDDEF(1200);
      BAUDDEF(1800);
      BAUDDEF(2400);
      BAUDDEF(4800);
      BAUDDEF(9600);
      BAUDDEF(19200);
      BAUDDEF(38400);
#ifdef B57600
      BAUDDEF(57600);
#endif
#ifdef B115200
      BAUDDEF(115200);
#endif
#ifdef B230400 
      BAUDDEF(230400);
#endif
#ifdef B460800 
      BAUDDEF(460800);
#endif
#ifdef B500000 
      BAUDDEF(500000);
#endif
#ifdef B576000
      BAUDDEF(576000);
#endif
#ifdef B921600
      BAUDDEF(921600);
#endif
#ifdef B1000000
      BAUDDEF(1000000);
#endif
#ifdef B1152000
      BAUDDEF(1152000);
#endif
#ifdef B1500000
      BAUDDEF(1500000);
#endif
#ifdef B2000000
      BAUDDEF(2000000);
#endif
#ifdef B2500000
      BAUDDEF(2500000);
#endif
#ifdef B3000000
      BAUDDEF(3000000);
#endif
#ifdef B3500000
      BAUDDEF(3500000);
#endif
#ifdef B4000000
      BAUDDEF(4000000);
#endif
#undef BAUDDEF
    default:
    #ifdef CBAUDEX
      baud = CBAUDEX;
      options.c_ispeed = baudRate;    // Set the input baud rate
      options.c_ospeed = baudRate;    // Set the output baud rate
      break;
    #else
    return -ENOTSUP;
    #endif
    }
    cfsetispeed(&options, baud);
    cfsetospeed(&options, baud);

    options.c_cflag = (options.c_cflag & (~CBAUD)) | baud;

    switch(parity)
    {
      case Parity::None:
        // No parity bit
        options.c_cflag &= ~(PARENB);
        // Disable parity check
        options.c_iflag &= ~INPCK;
      break;
      case Parity::Even:
        // Enable parity bit
        options.c_cflag |= PARENB;
        // Parity is not odd
        options.c_cflag &= ~PARODD;
        // Enable parity check
        options.c_iflag |= (INPCK | PARMRK);
        break;
      case Parity::Odd:
        // Enable parity bit
        options.c_cflag |= PARENB;
        // Parity is odd
        options.c_cflag |= PARODD;
        // Enable parity check
        options.c_iflag |= (INPCK);
        break;
      default:
        return -ENOTSUP;
    }

    if(stop == Stop::OneBit)
    {
      options.c_cflag &= ~CSTOPB;
    }
    else
    {
      options.c_cflag |= CSTOPB;
    }

    int size;
    switch(dataBits)
    {
      case 5:
      size = CS5;
      break;
      case 6:
      size = CS6;
      break;
      case 7:
      size = CS7;
      break;
      case 8:
      size = CS8;
      break;
      default:
      return -ENOTSUP;
    }

    if(dataBits < 8)
    {
      // If we're getting <8 data bits, strip parity bit and mark byte with invalid parity as 0xFF
      options.c_iflag |= (ISTRIP | PARMRK);
    }
    else
    {
      // If we get 8 data bits, there's no way to mark a parity error without escaping, so don't do it
      options.c_iflag &= ~(ISTRIP | PARMRK);
    }

    options.c_cflag = (options.c_cflag & ~CSIZE) | size;

    // Use RAW input mode, since this presumably is not a terminal
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    if(flowControl) options.c_cflag |= CRTSCTS;
    else options.c_cflag &= ~CRTSCTS;

    // Want raw output
    options.c_oflag &= ~OPOST;

    _timeout = timeout;

    // Timeout is in 100ms intervals, which is pretty inconvenient
    options.c_cc[VTIME] = timeout / 100;
    options.c_cc[VMIN] = 0;

    if(tcsetattr(_hPort, TCSAFLUSH, &options) != 0)
      return -errno;

    if (options.c_cc[VTIME] > 0)
    {
      if(fcntl(_hPort, F_SETFL, 0) != 0) return -errno;
    }
    else
    {
      // Set NDELAY to prevent reads from block if VTIME=0
      if(fcntl(_hPort, F_SETFL, FNDELAY) != 0) return -errno;
    }

    // Purge data in serial port
    return purge(Purge::All);
  }


  int Device::write(const void* data, unsigned count) noexcept
  {
    int written = ::write(_hPort, data, count);

    if (written == -1)
    {
      return -errno;
    }
    else
    {
      return written;
    }
  }

  int Device::read(void* data, unsigned count) noexcept
  {
    int ret = 1;
    if(_timeout < 100)
    {
      // Polling is required for timeouts < 100ms
      pollfd fds[1];
      fds[0].fd = _hPort;
      fds[0].events = POLLIN;
      ret = poll( fds, 1, _timeout);
    }

    if(ret > 0)
    {
      ret = ::read(_hPort, data, count);
    }
  
    if(ret < 0) return -errno;

    return ret;
  }

  int  Device::receiveQueueLevel() noexcept
  {
    int bytes;
    if(0 == ioctl(_hPort, FIONREAD, &bytes))
    {
      return bytes;
    }
    else
    {
      return -errno;
    }
  }

  int Device::transmitQueueLevel() noexcept
  {
    int bytes;
    if(0 == ioctl(_hPort, TIOCOUTQ, &bytes))
    {
      return bytes;
    }
    else
    {
      return -errno;
    }
  }

  int Device::events() noexcept
  {
    serial_icounter_struct counts;
    int ret = ioctl(_hPort, TIOCGICOUNT, &counts);
    if(ret != 0) return -errno;
    if(_parityEvents < counts.parity)
    {
      _parityEvents = counts.parity;
      ret |= static_cast<int>(Event::ParityError);
    }
    if(_frameEvents < counts.frame)
    {
      _frameEvents = counts.frame;
      ret |= static_cast<int>(Event::FrameError);
    }
    if(_breakEvents < counts.brk)
    {
      _breakEvents = counts.brk;
      ret |= static_cast<int>(Event::Break);
    }
    if(_overflowEvents < counts.overrun)
    {
      _overflowEvents = counts.overrun;
      ret |= static_cast<int>(Event::Overflow);
    }
    if(_overrunEvents < counts.buf_overrun)
    {
      _overrunEvents = counts.buf_overrun;
      ret |= static_cast<int>(Event::Overrun);
    }
    
    return ret;
  }

  int Device::flush() noexcept
  {
    int status = tcdrain(_hPort);
    if(status != 0) status = -errno;
    return status;
  }

  int Device::purge(Purge type) noexcept
  {
    int status = tcflush(_hPort, static_cast<int>(type));
    if(status != 0) status = -errno;
    return status;
  }

  int Device::close() noexcept
  {
    int error = 0;
    if (_hPort != INVALID_HANDLE_VALUE)
    {
      error = ::close(_hPort);
      if(error != 0) error = -errno;
      _hPort = INVALID_HANDLE_VALUE;
    }
    return error;
  }
}
