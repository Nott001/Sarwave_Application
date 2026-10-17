# Sarwave.DevEnv: the development front end -- port and cfg dialogues, the live run (RunStuff)
# and the recorded replay (RunRecorded) that host the Sarwave.Gui dashboards.
#
# QML module "Sarwave.DevEnv" without a plugin: DevEnv::initialize() registers its types.

qt_add_library(SarwaveDevEnv STATIC)

qt_add_qml_module(SarwaveDevEnv
    URI Sarwave.DevEnv
    VERSION 1.0
    NO_PLUGIN
    NO_IMPORT_SCAN
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml/Sarwave/DevEnv
    SOURCES
        DevEnv.hpp DevEnv.cpp
        ConfigMemory.hpp ConfigMemory.cpp
        TextBoxLayoutExtensions.hpp TextBoxLayoutExtensions.cpp
        Gui/DialogSupport.hpp Gui/DialogSupport.cpp
        Gui/PortDialogue.hpp Gui/PortDialogue.cpp
        Gui/CfgDialogue.hpp Gui/CfgDialogue.cpp
        Gui/ImuFrames.hpp Gui/ImuFrames.cpp
        Gui/Run.hpp Gui/RunRecorded.cpp Gui/RunStuff.cpp
    QML_FILES
        Gui/PortDialogueWindow.qml
        Gui/CfgDialogueWindow.qml
)

target_include_directories(SarwaveDevEnv
    PUBLIC ${CMAKE_SOURCE_DIR}/include
    # qmltyperegistrar includes the QML_ELEMENT headers by file name.
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Gui)

target_link_libraries(SarwaveDevEnv
    PUBLIC SarwaveCoreGui SarwaveCoreMMWave SarwaveCoreImu Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick
    PRIVATE Qt6::QuickControls2)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(SarwaveDevEnv PRIVATE -Wall -Wextra)
endif()
