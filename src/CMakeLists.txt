

set(SERIALPP_SOURCES ${PROJECT_SOURCE_DIR}/include/SerialEnumeration.hpp ${PROJECT_SOURCE_DIR}/include/Serial.hpp SerialEnumeration.cpp)
if(WIN32)
  list(APPEND SERIALPP_SOURCES SerialEnumerationWindows.cpp SerialWindows.cpp)
else()
  list(APPEND SERIALPP_SOURCES SerialEnumerationLinux.cpp SerialLinux.cpp)
endif()

# Add source to this project's executable.
add_library(serial STATIC ${SERIALPP_SOURCES}  )
target_include_directories(serial PUBLIC ${PROJECT_SOURCE_DIR}/include)

if(WIN32)
  set(LINK_TARGETS Cfgmgr32)
else()

  set(LINK_TARGETS udev)
endif()

target_link_libraries(serial PUBLIC ${LINK_TARGETS})

