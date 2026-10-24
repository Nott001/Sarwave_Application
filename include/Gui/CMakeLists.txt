# Sarwave.Core.Gui: the reusable radar dashboard (MinimalGui / MaximalGui), ported from Avalonia
# to Qt Quick. The C# folder layout is kept: Models/, ViewModels/, Views/, Styles/.
#
# C++ types are registered with QML_ELEMENT; every QML file belongs to the one module
# "Sarwave.Gui". The module has no plugin: Views::initialize() (and the show*Gui functions)
# register its types directly, which also pulls them out of this static library at link time.

set(GUI_DIR ${CMAKE_CURRENT_SOURCE_DIR})

qt_add_library(SarwaveCoreGui STATIC)

set_source_files_properties(Styles/Tokens.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(SarwaveCoreGui
    URI Sarwave.Gui
    VERSION 1.0
    NO_PLUGIN
    NO_IMPORT_SCAN
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml/Sarwave/Gui
    SOURCES
        Common/DotNetRandom.hpp Common/DotNetRandom.cpp
        Models/RadarData.hpp Models/RadarData.cpp
        ViewModels/ViewModelBase.hpp ViewModels/ViewModelBase.cpp
        ViewModels/StatusViewModel.hpp ViewModels/StatusViewModel.cpp
        ViewModels/MetricsViewModel.hpp ViewModels/MetricsViewModel.cpp
        ViewModels/MinimalGuiViewModel.hpp ViewModels/MinimalGuiViewModel.cpp
        ViewModels/MaximalGuiViewModel.hpp ViewModels/MaximalGuiViewModel.cpp
        ViewModels/Radar/RadarSnapshot.hpp ViewModels/Radar/RadarSnapshot.cpp
        ViewModels/Radar/RadarGridViewModel.hpp ViewModels/Radar/RadarGridViewModel.cpp
        ViewModels/Object/ObjectItemViewModel.hpp ViewModels/Object/ObjectItemViewModel.cpp
        ViewModels/Object/ObjectsPanelViewModel.hpp ViewModels/Object/ObjectsPanelViewModel.cpp
        Views/GuiHost.hpp Views/GuiHost.cpp
        Views/StringFormat.hpp Views/StringFormat.cpp
        Views/Object/ZRangeBar.hpp Views/Object/ZRangeBar.cpp
        Views/Radar/ColourManager.hpp Views/Radar/ColourManager.cpp
        Views/Radar/RadarGeometry.hpp Views/Radar/RadarGeometry.cpp
        Views/Radar/RadarGridPlot.hpp Views/Radar/RadarGridPlot.cpp
        Views/Radar/RadarDataOverlay.hpp Views/Radar/RadarDataOverlay.cpp
    QML_FILES
        Styles/Tokens.qml
        Views/MaximalGui.qml
        Views/MinimalGui.qml
        Views/StatusHeader.qml
        Views/Metrics/MetricCell.qml
        Views/Metrics/MetricsStrip.qml
        Views/Object/ObjectItem.qml
        Views/Object/ObjectsPanel.qml
        Views/Radar/RadarGrid.qml
        Views/Radar/SensorMarker.qml
)

# The embedded fonts: Inter (Avalonia's default UI font, as Avalonia.Fonts.Inter ships it) and
# Cascadia Mono (SarMonoFont). Registered by Views::initialize().
set(GUI_FONT_DIR ${CMAKE_SOURCE_DIR}/assets/Fonts)
set(GUI_FONTS
    Inter-Regular.ttf Inter-Medium.ttf Inter-SemiBold.ttf Inter-Bold.ttf Inter-Light.ttf
    Inter-Thin.ttf CascadiaMono.ttf)
list(TRANSFORM GUI_FONTS PREPEND ${GUI_FONT_DIR}/)
qt_add_resources(SarwaveCoreGui "SarwaveGuiFonts"
    PREFIX "/Sarwave/Gui/Fonts"
    BASE ${GUI_FONT_DIR}
    FILES ${GUI_FONTS})

target_include_directories(SarwaveCoreGui
    PUBLIC ${CMAKE_SOURCE_DIR}/include
    # qmltyperegistrar includes the QML_ELEMENT headers by file name.
    PRIVATE ${GUI_DIR}/ViewModels ${GUI_DIR}/ViewModels/Radar ${GUI_DIR}/ViewModels/Object
            ${GUI_DIR}/Views ${GUI_DIR}/Views/Radar ${GUI_DIR}/Views/Object)

target_link_libraries(SarwaveCoreGui
    PUBLIC SarwaveCoreMMWave SarwaveIntegrationPeopleTracking Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick
    PRIVATE Qt6::QuickControls2)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(SarwaveCoreGui PRIVATE -Wall -Wextra)
endif()

# Offscreen renderer that writes screenshots of both dashboards (design data and synthetic live
# frames). Only needed when checking the port against the AXAML, so off by default.
option(SARWAVE_BUILD_GUI_PREVIEW "Build the SarwaveGuiPreview screenshot tool" OFF)
if(SARWAVE_BUILD_GUI_PREVIEW)
    qt_add_executable(SarwaveGuiPreview Preview/main.cpp)
    target_link_libraries(SarwaveGuiPreview PRIVATE SarwaveCoreGui Qt6::Gui Qt6::Quick)
endif()
