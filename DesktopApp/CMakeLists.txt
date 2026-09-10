qt_add_executable(DesktopApp
    main.cpp
    ../common/dashboard_state.h
)

qt_add_qml_module(DesktopApp
    URI "DesktopApp"
    VERSION 1.0
    # The system Qt build uses shared QML plugins; they are resolved at runtime.
    # Avoid CMake trying to link non-exported plugin targets from the distro Qt package.
    NO_IMPORT_SCAN
    QML_FILES main.qml
)

target_link_libraries(DesktopApp PRIVATE common Qt6::Quick)

set_target_properties(DesktopApp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/DesktopApp)
