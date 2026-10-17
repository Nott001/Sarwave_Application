# Sarwave.Source.RecordedRpiApp: one bare window hosting the dev environment's run.
qt_add_executable(RecordedRpiApp main.cpp)
target_link_libraries(RecordedRpiApp PRIVATE SarwaveDevEnv SarwaveCoreGui Qt6::Quick)
set_target_properties(RecordedRpiApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    WIN32_EXECUTABLE TRUE)
