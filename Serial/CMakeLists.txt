

set(SERIALPP_SOURCES SerialEnumeration.h SerialEnumeration.cpp)
if(WIN32)
  list(APPEND SERIALPP_SOURCES SerialEnumerationWindows.cpp)
else()
  list(APPEND SERIALPP_SOURCES SerialEnumerationLinux.cpp)
endif()

# Add source to this project's executable.
add_library(serialpp SHARED ${SERIALPP_SOURCES})
target_compile_definitions(serialpp PRIVATE -DEXPORTING_SERIAL)

if(WIN32)
target_link_libraries(serialpp Cfgmgr32)
else()
  target_link_libraries(serialpp udev)
endif()

add_executable(SerialTest "main.cpp")
target_link_libraries(SerialTest serialpp)

