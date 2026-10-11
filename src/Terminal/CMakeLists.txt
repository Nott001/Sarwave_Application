# Sarwave.Source.Terminal
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Program.cpp)
    add_executable(SarwaveTerminal Program.cpp)
    target_link_libraries(SarwaveTerminal PRIVATE SarwaveCoreMMWave)
endif()
