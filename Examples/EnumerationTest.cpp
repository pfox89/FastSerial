#if defined(WIN32) && !defined(NDEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "SerialEnumeration.hpp"

#include <chrono>
#include <iostream>
#include <system_error>
#include <iomanip>
#include <sstream>

int main()
{
  {
    using std::chrono::high_resolution_clock;
    std::ostringstream ss;
    auto t1 = high_resolution_clock::now();
    for (auto& port : Serial::ports)
    {
      ss << port;
    }
    auto t2 = high_resolution_clock::now();
    std::cout << ss.str() << std::flush;
    auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    std::cout << "Took " << ms_int.count() << "ms" << std::endl;
  }
#if defined(WIN32) && !defined(NDEBUG)
  return _CrtDumpMemoryLeaks();
#else
  return 0;
#endif
}