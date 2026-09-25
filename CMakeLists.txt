cmake_minimum_required(VERSION 4.2)
project(SarwaveApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)

# QtQuick.Controls is imported by both QML entry points.  It must be found here
# so Qt can link and deploy its QML plugins along with each executable.
find_package(Qt6 REQUIRED COMPONENTS Core Network Quick QuickControls2 Test)
qt_standard_project_setup(REQUIRES 6.5)

# Enable compile_commands.json for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_subdirectory(include/MMWave)
add_subdirectory(include/QtCommon)
add_subdirectory(src/MMWaveTerminal)
add_subdirectory(src/RpiApp)
add_subdirectory(src/DesktopApp)
add_subdirectory(src/tests)

# Nonexistent test project on github.
option(BUILD_TEST_RPI_APP "Build TestRpiApp" ON)
if(BUILD_TEST_RPI_APP)
    # Check if the directory exists and contains a CMakeLists.txt
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/TestRpiApp/CMakeLists.txt")
        add_subdirectory(src/TestRpiApp)
    else()
        message(STATUS "Directory src/TestRpiApp not found. Skipping.")
    endif()
endif()