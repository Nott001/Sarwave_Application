cmake_minimum_required(VERSION 3.25)
project(SarwaveApp VERSION 0.2 LANGUAGES CXX)

# C++ port of the SarwaveApplication C# solution. Each C# project has a counterpart here:
#
#   Sarwave.Core.MMWave         -> include/MMWave   (SarwaveCoreMMWave, SarwaveCoreMMWaveConfig)
#   Sarwave.Core.Imu            -> include/Imu      (SarwaveCoreImu)
#   Sarwave.Core.Gui            -> include/Gui      (SarwaveCoreGui)
#   Sarwave.DevEnv              -> include/DevEnv   (SarwaveDevEnv)
#   Sarwave.Source.Terminal     -> src/Terminal     (SarwaveTerminal)
#   Sarwave.Source.TestRpiApp   -> src/TestRpiApp   (TestRpiApp)
#   Sarwave.Source.LiveRpiApp   -> src/LiveRpiApp   (LiveRpiApp)
#   Sarwave.Source.RecordedRpiApp -> src/RecordedRpiApp (RecordedRpiApp)

# Release unless told otherwise. A Debug build runs the per-frame work about five times slower,
# so it is for development only, never for the Pi (cmake -DCMAKE_BUILD_TYPE=Debug).
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Debug, Release, RelWithDebInfo or MinSizeRel" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# No fused multiply-add contraction: .NET never contracts a * b + c, and GCC does by default on
# aarch64 (the Raspberry Pi), which would make the float results differ from the C# app's in the
# last bit there but not on x86.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-ffp-contract=off)
endif()

# Code tuned for the CPU doing the build (the Pi 5's Cortex-A76 when built on the Pi); those
# binaries may not run on another machine. Measured gain is small (about 10% of the worker's
# already tiny per-frame time), so it is off by default. Results stay bit-identical to the C#
# app's: the flag changes instruction choice, not the order or rounding of float operations.
option(SARWAVE_NATIVE_CPU "Optimise for the CPU that builds it" OFF)
if(SARWAVE_NATIVE_CPU AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
        add_compile_options(-mcpu=native)
    else()
        add_compile_options(-march=native)
    endif()
endif()

# Link-time optimisation: the compiler sees the whole program at once, so it can inline across
# libraries (the MMWave and IMU code into the apps). Non-Debug builds only. Measured with
# SarwaveBench it made no difference beyond noise (worker ~19 us per frame either way; the paint
# time is spent inside Qt, which it doesn't touch), while a one-file rebuild went from ~3 s to
# ~40 s. So it is off by default; turn on with -DSARWAVE_LTO=ON.
option(SARWAVE_LTO "Link-time optimisation in non-Debug builds" OFF)
if(SARWAVE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT sarwave_ipo_supported OUTPUT sarwave_ipo_error LANGUAGES CXX)
    if(sarwave_ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL ON)
    else()
        message(STATUS "Link-time optimisation not supported here: ${sarwave_ipo_error}")
    endif()
endif()

# Enable compile_commands.json for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED CONFIG)
find_package(Threads REQUIRED)

option(SARWAVE_BUILD_GUI "Build the Qt based GUI libraries and applications" ON)

if(SARWAVE_BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Test)
    qt_standard_project_setup(REQUIRES 6.5)
    if(QT_KNOWN_POLICY_QTP0004)
        qt_policy(SET QTP0004 NEW)  # qmldir for the QML files in sub-folders of a module
    endif()
endif()

enable_testing()

add_subdirectory(include/MMWave)
add_subdirectory(include/Imu)
add_subdirectory(src/Terminal)

if(SARWAVE_BUILD_GUI)
    add_subdirectory(include/Gui)
    add_subdirectory(include/DevEnv)

    option(BUILD_TEST_RPI_APP "Build TestRpiApp" ON)
    if(BUILD_TEST_RPI_APP)
        add_subdirectory(src/TestRpiApp)
    endif()

    # The two apps the Raspberry Pi runs: live from the sensor (recording), and the replay.
    add_subdirectory(src/LiveRpiApp)
    add_subdirectory(src/RecordedRpiApp)

    # Times the per-frame work on a recording (run it on the Pi).
    add_subdirectory(src/bench)
endif()

add_subdirectory(src/tests)
