add_executable(EnumerationTest "EnumerationTest.cpp")
target_link_libraries(EnumerationTest fastserial)

add_executable(LoopbackTest "LoopbackTest.cpp")
target_link_libraries(LoopbackTest fastserial)