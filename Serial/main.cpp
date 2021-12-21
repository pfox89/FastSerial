#include "SerialEnumeration.h"

#include <chrono>
#include <iostream>
#include <system_error>


int main()
{
  using std::chrono::high_resolution_clock;

    //COMApartment apartment;
    auto t1 = high_resolution_clock::now();

    for (auto& port : Serial::ports)
    {
      std::cout << "Path: " << port.path << ", "
        << "Type: " << port.type << ", "
        << "Manufacturer: " << port.manufacturer << ", "
        << "Name: " << port.name << ", "
        << "Description: " << port.description << ", "
        << "VID: " << port.vid << ", " << "PID: " << port.pid << "\r\n";
    }
    auto t2 = high_resolution_clock::now();
    std::cout << std::flush;
    auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    std::cout << "Took " << ms_int.count() << "ms" << std::endl;


  return 0;
}