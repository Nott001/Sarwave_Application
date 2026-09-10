cmake_minimum_required(VERSION 3.16)
project(DesktopApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

qt_add_executable(DesktopApp
    main.cpp
)

qt_add_qml_module(DesktopApp
    URI "DesktopApp"
    VERSION 1.0
    QML_FILES main.qml
)

target_link_libraries(DesktopApp PRIVATE common Qt6::Quick)

set_target_properties(DesktopApp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/DesktopApp)