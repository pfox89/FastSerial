#include <iostream>
#include <chrono>
#include <cstring>

using std::chrono::high_resolution_clock;

#include "Serial.hpp"

static const char testpattern[] = "TestPattern";
static char buffer[sizeof(testpattern)] = { 0 };

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <Port>" << std::endl;
    return -1;
  }

  // --- Open serial port ---
  Serial::Device dev;
  int status = dev.open(argv[1]);
  if (status < 0)
  {
    std::cerr << "Error opening port " << argv[1] << ": " << std::error_code(-status, std::system_category()).message() << std::endl;
    return -2;
  }
  std::cout << "Port open" << std::endl;

  // --- Configure baud rate, mode, data bits, stop bits, parity, and flow control ---
  status = dev.configure(9600, 8, Serial::Stop::OneBit, Serial::Parity::None, false, 20);

  if (status < 0)
  {
    std::cerr << "Error configuring port: " << std::error_code(-status, std::system_category()).message() << std::endl;
    return -3;
  }
  std::cout << "Port configured" << std::endl;

  // --- Write data to port ---
  auto t1 = high_resolution_clock::now();
  status = dev.write(testpattern, sizeof(testpattern));
  auto t2 = high_resolution_clock::now();

  if (status < 0)
  {
    std::cerr << "Error writing to port: " << std::error_code(-status, std::system_category()).message() << std::endl;
    return -4;
  }

  auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
  std::cout << "Wrote " << status << " bytes in " << ms_int.count() << "ms" << std::endl;

  if (status != sizeof(testpattern))
  {
    std::cerr << "Write to port was incomplete: " << sizeof(testpattern) << " bytes requested " << status << " bytes written" << std::endl;
    return -4;
  }

  /*
  t1 = high_resolution_clock::now();
  status = dev.flush();
  t2 = high_resolution_clock::now();

  if (status < 0)
  {
    std::cerr << "Error flusing data: " << std::error_code(-status, std::system_category()).message() << std::endl;
    return -4;
  }
  ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
  std::cout << "Flush took " << ms_int.count() << "ms" << std::endl;
  */

  // --- Read data from port ---
  t1 = high_resolution_clock::now();
  Serial::Frame<sizeof(buffer)> to_read(dev);
  while((status = to_read.read()) == 0)
  {

  }
  t2 = high_resolution_clock::now();
  if (status < 0)
  {
    std::cerr << "Error reading from port: " << std::error_code(-status, std::system_category()).message() << std::endl;
    return -5;
  }
  ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
  std::cout << "Read " << status << " bytes in " << ms_int.count() << "ms" << std::endl;

  if (status != sizeof(testpattern))
  {
    std::cerr << "Error receiving bytes " << sizeof(testpattern) << " bytes sent " << status << " bytes received" << std::endl;
    return -5;
  }

  // --- Check data that was read back ---

  if (memcmp(testpattern, to_read.data(), sizeof(testpattern)) != 0)
  {
    std::cerr << "Data received does not match! Sent " << testpattern << " received " << buffer << std::endl;
    return -6;
  }
  std::cout << "OK" << std::endl;
  return 0;
}