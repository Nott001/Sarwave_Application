# Sarwave.Integration.PeopleTracking: the People Tracking output put together with the IMU --
# the targets and point clouds a session holds, moved with the device as it turns -- the IMU glue
# the apps share (ImuFrames), and streaming a session over the LAN (Lan/, on Boost.Asio).
# Pure C++23, no Qt, like the two libraries it joins.

file(GLOB_RECURSE INTEGRATION_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/PeopleTracking/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PeopleTracking/*.hpp)

add_library(SarwaveIntegrationPeopleTracking STATIC ${INTEGRATION_SOURCES})
target_include_directories(SarwaveIntegrationPeopleTracking PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(SarwaveIntegrationPeopleTracking
    PUBLIC SarwaveCoreMMWave SarwaveCoreMMWaveConfig SarwaveCoreImu)
set_target_properties(SarwaveIntegrationPeopleTracking PROPERTIES
    AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(SarwaveIntegrationPeopleTracking PRIVATE -Wall -Wextra)
endif()
