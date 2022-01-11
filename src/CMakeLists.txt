

set(SERIALPP_SOURCES ${PROJECT_SOURCE_DIR}/include/SerialEnumeration.h ${PROJECT_SOURCE_DIR}/include/Serial.hpp SerialEnumeration.cpp)
if(WIN32)
  list(APPEND SERIALPP_SOURCES SerialEnumerationWindows.cpp SerialWindows.cpp)
else()
  list(APPEND SERIALPP_SOURCES SerialEnumerationLinux.cpp SerialLinux.cpp)
endif()

# Add source to this project's executable.
add_library(fastserial SHARED ${SERIALPP_SOURCES})
target_compile_definitions(fastserial PRIVATE -DEXPORTING_SERIAL -DEXPORTING_SERIALENUM)
target_include_directories(fastserial PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_library(fastserial_static STATIC ${SERIALPP_SOURCES})
target_compile_definitions(fastserial_static PUBLIC -DSERIAL_STATIC)
target_include_directories(fastserial_static PUBLIC ${PROJECT_SOURCE_DIR}/include)

if(WIN32)
  set(LINK_TARGETS Cfgmgr32)
else()

  set(LINK_TARGETS udev)
endif()

target_link_libraries(fastserial PUBLIC ${LINK_TARGETS})
target_link_libraries(fastserial_static PUBLIC ${LINK_TARGETS})

