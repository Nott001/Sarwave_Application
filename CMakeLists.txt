cmake_minimum_required(VERSION 3.16)
project(SarwaveApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Core Network)
qt_standard_project_setup(REQUIRES 6.5)

# Enable compile_commands.json for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(common)
add_subdirectory(RpiApp)
add_subdirectory(DesktopApp)
add_subdirectory(tests)
