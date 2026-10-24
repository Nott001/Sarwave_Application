# Sarwave.Source.WirelessRpiApp: one bare window hosting the dev environment's run.
qt_add_executable(WirelessRpiApp main.cpp)
target_link_libraries(WirelessRpiApp PRIVATE SarwaveDevEnv SarwaveCoreGui Qt6::Quick)
set_target_properties(WirelessRpiApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    WIN32_EXECUTABLE TRUE)
