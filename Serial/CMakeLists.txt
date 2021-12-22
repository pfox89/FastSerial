# CMakeList.txt : CMake project for SerialEnumeration, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_library(serialpp SHARED "SerialEnumerationWindows.cpp" "SerialEnumeration.h" "SerialEnumeration.cpp")
target_compile_definitions(serialpp PRIVATE -DEXPORTING_SERIAL)
target_link_libraries(serialpp Cfgmgr32)

add_executable(SerialTest "main.cpp")
target_link_libraries(SerialTest serialpp)
# TODO: Add tests and install targets if needed.
