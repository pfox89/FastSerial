#if defined(WIN32) && !defined(NDEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif

#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>

using std::chrono::high_resolution_clock;

#include "Serial.hpp"

static const char testpattern[]               = "TestPattern";
static char       buffer[sizeof(testpattern)] = { 0 };

int pusage(const char* name)
{
  std::cerr << "Usage: " << name << " <Port> <timeout_ms>" << std::endl;
  return -1;
}

int main(int argc, char** argv)
{
  if (argc < 3) { return pusage(argv[0]); }

  unsigned short timeout = 0;
  {
    unsigned long ultimeout = std::strtoul(argv[2], nullptr, 10);
    if (ultimeout == 0) { return pusage(argv[0]); }
    if (ultimeout > USHRT_MAX)
    {
      std::cerr << "Timeout " << ultimeout << " too long" << std::endl;
      return -1;
    }
    timeout = static_cast<unsigned short>(ultimeout);
  }

  // --- Open serial port ---
  Serial::Device dev;
  int            status = dev.open(argv[1]);
  if (status < 0)
  {
    std::cerr << "Error opening port " << argv[1] << ": " << std::error_code(-status, std::system_category()).message()
              << std::endl;
    return -2;
  }
  std::cout << "Port open" << std::endl;

  // --- Configure baud rate, mode, data bits, stop bits, parity, and flow control ---
  status = dev.configure(9600, 8, Serial::Device::Stop::OneBit, Serial::Device::Parity::None, false, timeout);

  if (status < 0)
  {
    std::cerr << "Error configuring port: " << std::error_code(-status, std::system_category()).message() << std::endl;
    return -3;
  }
  std::cout << "Port configured" << std::endl;

  // --- Write data to port ---
  auto t1 = high_resolution_clock::now();
  // Intentionally send 1 byte less to trigger timeout
  status  = dev.write(testpattern, sizeof(testpattern) - 1);
  auto t2 = high_resolution_clock::now();

  if (status < 0)
  {
    std::cerr << "Error writing to port: " << std::error_code(-status, std::system_category()).message() << std::endl;
    return -4;
  }

  auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
  std::cout << "Wrote " << status << " bytes in " << ms_int << "ms" << std::endl;

  if (status != sizeof(testpattern) - 1)
  {
    std::cerr << "Write to port was incomplete: " << sizeof(testpattern) << " bytes requested " << status
              << " bytes written" << std::endl;
    return -4;
  }

  // --- Read data from port ---
  int total_read = 0;
  t1             = high_resolution_clock::now();
  do {
    status = dev.read(&buffer[total_read], sizeof(buffer) - total_read);
    if (status > 0) total_read += status;
  } while (status > 0 && total_read < sizeof(buffer));

  t2 = high_resolution_clock::now();

  if (status == 0)
  {
    if (total_read != sizeof(testpattern) - 1)
    {
      std::cerr << "Error receiving bytes " << sizeof(testpattern) - 1 << " bytes sent " << total_read
                << " bytes received" << std::endl;
      return -5;
    }
    else
    {
      ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
      if (ms_int > static_cast<decltype(ms_int)>(timeout) + 50)
      {
        std::cerr << "Took too long to time out! Expected " << static_cast<decltype(ms_int)>(timeout) + 50 << "ms, took " << ms_int << "ms"
                  << std::endl;
        return -6;
      }
      std::cout << "Successfully timeout out after reading " << total_read << " bytes in " << ms_int << "ms"
                << std::endl;
    }
  }
  else if (status < 0)
  {
    std::cerr << "Error reading from port: " << std::error_code(-status, std::system_category()).message() << std::endl;
    return -5;
  }
  else
  {
    std::cerr << "Expected incomplete read, check that the port is configured for loopback" << std::endl;
    return -5;
  }

  // --- Check data that was read back ---

  if (memcmp(testpattern, buffer, sizeof(testpattern) - 1) != 0)
  {
    std::cerr << "Data received does not match! Sent " << testpattern << " received " << buffer << std::endl;
    return -7;
  }
  std::cout << "OK" << std::endl;
#if defined(WIN32) && !defined(NDEBUG)
  return _CrtDumpMemoryLeaks();
#else
  return 0;
#endif
}