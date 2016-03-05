add_executable(test_basic basic.cpp)
target_link_libraries(test_basic ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${Boost_SYSTEM_LIBRARY})
add_test(NAME test_basic COMMAND test_basic)
