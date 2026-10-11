# One test executable per library, each built from src/tests/<Library>/*.cpp with the small
# dependency-free framework in TestFramework.hpp. Run them all with: ctest --test-dir build

function(sarwave_add_test_suite name dir)
    file(GLOB suite_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp)
    if(NOT suite_sources)
        return()
    endif()
    add_executable(${name} ${CMAKE_CURRENT_SOURCE_DIR}/TestMain.cpp ${suite_sources})
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${name} PRIVATE ${ARGN})
    target_compile_definitions(${name} PRIVATE
        SARWAVE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        SARWAVE_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
    add_test(NAME ${name} COMMAND ${name})
endfunction()

sarwave_add_test_suite(CoreTests Core SarwaveCoreMMWave)
sarwave_add_test_suite(ConfigTests Config SarwaveCoreMMWaveConfig)
if(SARWAVE_BUILD_GUI)
    sarwave_add_test_suite(GuiTests Gui SarwaveCoreGui SarwaveDevEnv Qt6::Test)
endif()

# Prints the decoded state of recordings in a fixed text form; the same program exists in C#
# (see src/tests/Parity/README.md), and the two outputs are diffed to prove the port matches.
add_executable(CoreParityHarness ${CMAKE_CURRENT_SOURCE_DIR}/Parity/CoreParityHarness.cpp)
target_link_libraries(CoreParityHarness PRIVATE SarwaveCoreMMWave)

# The .cfg model's parity harness (see Parity/README.md).
add_executable(ConfigParityHarness ${CMAKE_CURRENT_SOURCE_DIR}/Parity/ConfigParityHarness.cpp)
target_link_libraries(ConfigParityHarness PRIVATE SarwaveCoreMMWaveConfig)
