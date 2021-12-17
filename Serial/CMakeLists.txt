# CMakeList.txt : CMake project for SerialEnumeration, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_library(SerialXP SHARED "SerialEnumerationWindows.cpp" "SerialEnumeration.h")
target_link_libraries(SerialXP setupapi)

add_executable(SerialTest "main.cpp")
target_link_libraries(SerialTest SerialXP)
# TODO: Add tests and install targets if needed.
