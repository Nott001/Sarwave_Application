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

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# No fused multiply-add contraction: .NET never contracts a * b + c, and GCC does by default on
# aarch64 (the Raspberry Pi), which would make the float results differ from the C# app's in the
# last bit there but not on x86.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-ffp-contract=off)
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
endif()

add_subdirectory(src/tests)
