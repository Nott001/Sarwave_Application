# SarwaveBench: times the live app's per-frame work (reader, worker, paint) on a recording.
# Run it on the Raspberry Pi; see the comment at the top of PipelineBench.cpp.

add_executable(SarwaveBench PipelineBench.cpp)
target_link_libraries(SarwaveBench PRIVATE SarwaveDevEnv SarwaveCoreGui SarwaveCoreImu
    SarwaveCoreMMWave Qt6::Gui)
set_target_properties(SarwaveBench PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
