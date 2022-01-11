# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project (FastSerial)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

enable_testing()
if(LOOPBACK_SERIALPORT)
set(LOOPBACK_SERIALPORT "${LOOPBACK_SERIALPORT}" CACHE STRING "Ports with loopback connections to test with")
message(STATUS "Serial Port Selected: $CACHE{LOOPBACK_SERIALPORT}")
option(LOOPBACK_TESTS "Include loopback tests. Requires LOOPBACK_SERIALPORT to be specified" ON)
else()
message(WARNING "No serial port selected for tests. Use LOOPBACK_SERIALPORT to set serial port with loopback connection.")
option(LOOPBACK_TESTS "Include loopback tests. Requires LOOPBACK_SERIALPORT to be specified" OFF)
endif()

# Include sub-projects.
add_subdirectory (src)
add_subdirectory(Examples)
