# Sarwave.Source.Terminal
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Program.cpp)
    add_executable(SarwaveTerminal Program.cpp ImuMonitor.hpp ImuMonitor.cpp)
    target_link_libraries(SarwaveTerminal
        PRIVATE SarwaveCoreMMWave SarwaveCoreImu SarwaveIntegrationPeopleTracking)
endif()
