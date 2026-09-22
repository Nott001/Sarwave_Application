find_package(Qt6 REQUIRED COMPONENTS Qml Quick QuickControls2)

qt_add_library(MMWavePrompt STATIC
        PortDialog.hpp
        PortDialog.cpp
        CfgDialog.hpp
        CfgDialog.cpp
)

target_include_directories(MMWavePrompt PUBLIC ${CMAKE_SOURCE_DIR}/include)

set(MMWAVE_PROMPT_FONT ${CMAKE_SOURCE_DIR}/assets/Fonts/CascadiaMono.ttf)
set_source_files_properties(${MMWAVE_PROMPT_FONT}
    PROPERTIES QT_RESOURCE_ALIAS "fonts/CascadiaMono.ttf"
)

qt_add_resources(MMWavePrompt "MMWavePromptResources"
    PREFIX "/QtCommon/MMWavePrompt"
    FILES
        PortDialog.qml
        CfgDialog.qml
        ${MMWAVE_PROMPT_FONT}
)

target_link_libraries(MMWavePrompt PUBLIC Qt6::Qml Qt6::Quick)
