# Sarwave.Source.RecordedRpiApp: one empty window that the dev environment's RunRecorded fills.
qt_add_executable(RecordedRpiApp main.cpp)

target_link_libraries(RecordedRpiApp PRIVATE SarwaveDevEnv SarwaveCoreGui Qt6::Quick Qt6::QuickControls2)

set_target_properties(RecordedRpiApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    WIN32_EXECUTABLE TRUE)
