add_library(common STATIC
    dashboard_state.cpp
    dashboard_state.h
    network_manager.cpp
    protocol_definitions.cpp
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common PUBLIC Qt6::Core Qt6::Network)
