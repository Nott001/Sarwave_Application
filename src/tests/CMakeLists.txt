add_executable(tests test_main.cpp)
target_link_libraries(tests PRIVATE QtCommon Qt6::Core)

add_test(NAME SarwaveTests COMMAND tests)