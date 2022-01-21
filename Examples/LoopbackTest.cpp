#if defined(WIN32) && !defined(NDEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif

#include <chrono>
#include <cstring>
#include <iostream>

using std::chrono::steady_clock;

#include "Serial.hpp"

static const char testpattern[]               = "TestPattern";
static char       buffer[sizeof(testpattern)] = { 0 };

static Serial::Device dev;

typedef steady_clock::rep                        TimeRep;
typedef steady_clock::duration                   Duration;
typedef steady_clock                             Clock;
typedef std::chrono::time_point<Clock, Duration> TimePoint;

static auto get_ms(const TimePoint& start, const TimePoint& end) noexcept
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

static constexpr int sync_timeout = 50;

static int     total_read   = 0;
static TimeRep max_blocking = 0;

int readAsync(const TimePoint& t1) noexcept
{
  int status;

  do {
    auto ts = steady_clock::now();
    status  = dev.read(&buffer[total_read], sizeof(buffer) - total_read);
    auto te = steady_clock::now();
    if (status > 0) total_read += status;
    else if (get_ms(t1, ts) > 50)
    {
      // After 50ms since we started, we give up
      break;
    }
    // Ensure that it didn't block for too long
    auto ms = get_ms(ts, te);
    if (ms > max_blocking) max_blocking = ms;
    if (ms > 10)
    {
      std::cerr << "Async read took too long! Required <10ms, took " << ms << "ms" << std::endl;
      exit(-5);
    }
  } while (status >= 0 && total_read < sizeof(buffer));
  return status < 0 ? status : total_read;
}

int readSync() noexcept
{
  int status;

  int total_read = 0;
  do {
    auto ts = steady_clock::now();
    status  = dev.read(&buffer[total_read], sizeof(buffer) - total_read);
    auto te = steady_clock::now();
    if (status > 0) total_read += status;
    // Ensure that it didn't block for too long
    auto ms = get_ms(ts, te);
    if (ms > max_blocking) max_blocking = ms;
    if (ms > sync_timeout + 50)
    {
      std::cerr << "Read took too long! Required <" << sync_timeout + 50 << "ms, took " << ms << "ms" << std::endl;
      exit(-5);
    }
  } while (status > 0 && total_read < sizeof(buffer));
  return status < 0 ? status : total_read;
}

int pusage(const char* name)
{
  std::cerr << "Usage: " << name << " <Port> <sync/async>" << std::endl;
  return -1;
}

int main(int argc, char** argv)
{
  if (argc < 3) { return pusage(argv[0]); }
  bool async;
  if (strcmp(argv[2], "sync") == 0) { async = false; }
  else if (strcmp(argv[2], "async") == 0)
  {
    async = true;
  }
  else
  {
    return pusage(argv[0]);
  }

  // --- Open serial port ---
  int status = dev.open(argv[1]);
  if (status < 0)
  {
    std::cerr << "Error opening port " << argv[1] << ": " << std::error_code(-status, std::system_category()).message()
              << std::endl;
    return -2;
  }
  std::cout << "Port open" << std::endl;

  // --- Configure baud rate, mode, data bits, stop bits, parity, and flow control ---
  status = dev.configure(9600, 8, Serial::Stop::OneBit, Serial::Parity::None, false, async ? 0 : sync_timeout);

  if (status < 0)
  {
    std::cerr << "Error configuring port: " << std::error_code(-status, std::system_category()).message() << std::endl;
    return -3;
  }
  std::cout << "Port configured" << std::endl;

  // --- Write data to port ---
  auto t1 = Clock::now();
  status  = dev.write(testpattern, sizeof(testpattern));
  auto t2 = Clock::now();

  if (status < 0)
  {
    std::cerr << "Error writing to port: " << std::error_code(-status, std::system_category()).message() << std::endl;
    return -4;
  }

  auto ms_int = get_ms(t1, t2);
  std::cout << "Wrote " << status << " bytes in " << ms_int << "ms" << std::endl;

  if (status != sizeof(testpattern))
  {
    std::cerr << "Write to port was incomplete: " << sizeof(testpattern) << " bytes requested " << status
              << " bytes written" << std::endl;
    return -4;
  }

  // --- Read data from port ---

  t1 = Clock::now();
  if (async) { status = readAsync(t1); }
  else
  {
    status = readSync();
  }
  t2 = Clock::now();
  if (status < 0)
  {
    std::cerr << "Error reading from port: " << std::error_code(-status, std::system_category()).message() << std::endl;
    return -5;
  }
  ms_int = get_ms(t1, t2);

  std::cout << "Read " << status << " bytes in " << ms_int << "ms" << '\n'
            << "Max blocking time " << max_blocking << "ms" << std::endl;

  if (status != sizeof(testpattern))
  {
    std::cerr << "Error receiving bytes " << sizeof(testpattern) << " bytes sent " << status << " bytes received"
              << std::endl;
    return -5;
  }

  // --- Check data that was read back ---

  if (memcmp(testpattern, buffer, sizeof(testpattern)) != 0)
  {
    std::cerr << "Data received does not match! Sent " << testpattern << " received " << buffer << std::endl;
    return -6;
  }
  std::cout << "OK" << std::endl;
#if defined(WIN32) && !defined(NDEBUG)
  return _CrtDumpMemoryLeaks();
#else
  return 0;
#endif
}