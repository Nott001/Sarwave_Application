# Sarwave.Source.TestRpiApp: one window hosting the dev environment's run.
qt_add_executable(TestRpiApp main.cpp)

qt_add_qml_module(TestRpiApp
    URI TestRpiApp
    VERSION 1.0
    NO_IMPORT_SCAN
    QML_FILES MainWindow.qml
)

# The window icon of the C# app (Assets/avalonia-logo.ico).
qt_add_resources(TestRpiApp "TestRpiAppAssets"
    PREFIX "/TestRpiApp"
    BASE ${CMAKE_SOURCE_DIR}/assets/Icons
    FILES ${CMAKE_SOURCE_DIR}/assets/Icons/avalonia-logo.ico)

target_link_libraries(TestRpiApp PRIVATE SarwaveDevEnv SarwaveCoreGui Qt6::Quick Qt6::QuickControls2)

set_target_properties(TestRpiApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    WIN32_EXECUTABLE TRUE)
