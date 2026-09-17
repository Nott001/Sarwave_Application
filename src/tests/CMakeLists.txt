set(TEST_SOURCES
    test_main.cpp
)

qt_add_executable(tests ${TEST_SOURCES})
target_include_directories(tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/QtCommon
    ${CMAKE_SOURCE_DIR}/include/MMWave
    ${CMAKE_SOURCE_DIR}/include/MMWave/Prompting
)
target_link_libraries(tests PRIVATE QtCommon MMWave MMWavePrompt Qt6::Core Qt6::Test Boost::boost)

add_test(NAME SarwaveTests COMMAND tests)