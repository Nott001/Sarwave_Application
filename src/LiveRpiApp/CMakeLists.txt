# Sarwave.Source.LiveRpiApp: one empty window that the dev environment's RunStuff fills.
qt_add_executable(LiveRpiApp main.cpp)

target_link_libraries(LiveRpiApp PRIVATE SarwaveDevEnv SarwaveCoreGui Qt6::Quick Qt6::QuickControls2)

set_target_properties(LiveRpiApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    WIN32_EXECUTABLE TRUE)
