qt_add_library(QtCommon STATIC
    DashboardState.cpp
    DashboardState.h
    NetworkManager.cpp
    ProtocolDefinitions.cpp
)

target_include_directories(QtCommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(QtCommon PUBLIC Qt6::Core Qt6::Network)

add_subdirectory(MMWavePrompt)
