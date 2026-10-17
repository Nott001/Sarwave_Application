# Sarwave.Source.LiveRpiApp: one bare window hosting the dev environment's run.
qt_add_executable(LiveRpiApp main.cpp)
target_link_libraries(LiveRpiApp PRIVATE SarwaveDevEnv SarwaveCoreGui Qt6::Quick)
set_target_properties(LiveRpiApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    WIN32_EXECUTABLE TRUE)
