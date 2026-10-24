# The check tools for the device (the C# repo's tools/*.cs scripts): what the IMU read while held
# still (docs/imu_mounting_checks.md) and how many points a person gives in a pilot recording
# (docs/pilot_check.md). No Qt.

foreach(tool ImuCheck PilotCheck)
    add_executable(Sarwave${tool} ${tool}.cpp ToolSupport.hpp)
    target_link_libraries(Sarwave${tool} PRIVATE SarwaveIntegrationPeopleTracking)
    set_target_properties(Sarwave${tool} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endforeach()
