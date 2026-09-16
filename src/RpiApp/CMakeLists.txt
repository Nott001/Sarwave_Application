qt_add_executable(RpiApp
    main.cpp
    ../../include/QtCommon/DashboardState.h
)

qt_add_qml_module(RpiApp
    URI "RpiApp"
    VERSION 1.0
    # The system Qt build uses shared QML plugins; they are resolved at runtime.
    # Avoid CMake trying to link non-exported plugin targets from the distro Qt package.
    NO_IMPORT_SCAN
    QML_FILES main.qml
)

target_link_libraries(RpiApp PRIVATE QtCommon Qt6::Quick)

set_target_properties(RpiApp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/RpiApp)
