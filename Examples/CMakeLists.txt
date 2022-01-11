add_executable(EnumerationTest "EnumerationTest.cpp")
target_link_libraries(EnumerationTest fastserial)

add_test(NAME EnumerationTest COMMAND EnumerationTest)
set_tests_properties(EnumerationTest PROPERTIES TIMEOUT 10)

add_executable(LoopbackTest "LoopbackTest.cpp")
target_link_libraries(LoopbackTest fastserial)

add_executable(TimeoutTest "TimeoutTest")
target_link_libraries(TimeoutTest fastserial)


if(LOOPBACK_TESTS)
add_test(NAME Loopback COMMAND LoopbackTest $CACHE{LOOPBACK_SERIALPORT} sync)
set_tests_properties(Loopback PROPERTIES TIMEOUT 10)
add_test(NAME LoopbackAsync COMMAND LoopbackTest $CACHE{LOOPBACK_SERIALPORT} async)
set_tests_properties(LoopbackAsync PROPERTIES TIMEOUT 10)
add_test(NAME ShortTimeoutTest COMMAND TimeoutTest $CACHE{LOOPBACK_SERIALPORT} 20)
set_tests_properties(ShortTimeoutTest PROPERTIES TIMEOUT 10)
add_test(NAME LongTimeoutTest COMMAND TimeoutTest $CACHE{LOOPBACK_SERIALPORT} 200)
set_tests_properties(LongTimeoutTest PROPERTIES TIMEOUT 10)
endif()