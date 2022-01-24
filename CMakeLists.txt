# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project (FastSerial
        DESCRIPTION "Simple, cross-platform library for interacting with serial ports in C++")

find_package(Doxygen)
if(DOXYGEN_FOUND)
  option(CREATE_DOCS "Create documentation" ON)

  get_filename_component(DOXYGEN_PATH ${DOXYGEN_EXECUTABLE} DIRECTORY)
  find_program(DOXYBOOK_EXECUTABLE 
              doxybook2 
              HINTS ${DOXYGEN_PATH}
              )
  if(DOXYBOOK_EXECUTABLE)
    add_executable(Doxygen::doxybook IMPORTED GLOBAL)
    set_target_properties(Doxygen::doxybook PROPERTIES IMPORTED_LOCATION "${DOXYBOOK_EXECUTABLE}")
  else()
    message(WARNING "Unable to find doxybook. Please Download doxybook2.exe and place in Doxygen bin dir")
  endif()

else()
  message(WARNING "Doxygen not found, no documentation will be produced")
  option(CREATE_DOCS "Create documentation" OFF)
endif()

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
add_subdirectory(src)
add_subdirectory(Examples)

if(CREATE_DOCS)
  set(DOXYGEN_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/docs)
  set(DOXYGEN_GENERATE_XML YES)
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/ReadMe.md")

  if(DOXYGEN_DOT_PATH)
  set(DOXYGEN_DOT_IMAGE_FORMAT "svg")
  set(DOXYGEN_INTERACTIVE_SVG YES)
  else()
  message(WARNING "GraphViz not found. Please install GraphViz for documentation.")
  endif()
doxygen_add_docs(
  documentation 
  include/Serial.hpp include/SerialEnumeration.hpp 
  ${DOXYGEN_USE_MDFILE_AS_MAINPAGE}
  ALL 
  USE_STAMP_FILE
  COMMENT "Creating documentation")

  if(DOXYBOOK_EXECUTABLE)
    add_custom_command(
    TARGET documentation
    POST_BUILD
  #    OUTPUT ${DOXYGEN_OUTPUT_DIRECTORY}/markdown/index_classes.md
      COMMAND Doxygen::doxybook 
        --input ${DOXYGEN_OUTPUT_DIRECTORY}/xml 
        --output ${PROJECT_SOURCE_DIR}/Documentation
        -c ${PROJECT_SOURCE_DIR}/.doxybook/config.json
        -t ${PROJECT_SOURCE_DIR}/.doxybook/templates
      MAIN_DEPENDENCY ${DOXYGEN_OUTPUT_DIRECTORY}/Doxyfile.xml
      COMMENT "Creating markdown documentation"
    )
  endif()
endif()
