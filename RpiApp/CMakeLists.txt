cmake_minimum_required(VERSION 3.16)
project(RpiApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

qt_add_executable(RpiApp
    main.cpp
)

qt_add_qml_module(RpiApp
    URI "RpiApp"
    VERSION 1.0
    # The system Qt build uses shared QML plugins; they are resolved at runtime.
    # Avoid CMake trying to link non-exported plugin targets from the distro Qt package.
    NO_IMPORT_SCAN
    QML_FILES main.qml
)

target_link_libraries(RpiApp PRIVATE common Qt6::Quick)

set_target_properties(RpiApp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/RpiApp)
