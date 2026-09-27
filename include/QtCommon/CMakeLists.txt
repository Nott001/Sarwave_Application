qt_add_library(QtCommon STATIC
    Dashboard/DashboardState.cpp
    Dashboard/DashboardState.h
    NetworkManager.cpp
    NetworkManager.h
    ProtocolDefinitions.cpp
    ProtocolDefinitions.h
)

target_include_directories(QtCommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(QtCommon PUBLIC Qt6::Core Qt6::Network MMWave)

add_subdirectory(MMWavePrompt)
