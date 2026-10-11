// Design tokens (Tokens.axaml). Edit colours here; every view reads them from this singleton.
pragma Singleton
import QtQuick

QtObject {
    readonly property color windowBrush: "#09111f"
    readonly property color surface: "#111d31"
    readonly property color raised: "#172640"
    readonly property color border: "#1e3052"
    readonly property color primary: "#f4f7fb"
    readonly property color muted: "#91a4c2"
    readonly property color accent: "#31d0aa"
    readonly property color danger: "#ff4757"
    readonly property color blue: "#73a9ff"
    readonly property color purple: "#b9a5ff"
    readonly property color red: "#ff6b81"

    // Radar grid (alpha is the first byte: #AARRGGBB)
    readonly property color plotBackground: "#050d18"
    readonly property color background: "#030912"

    readonly property color sectorFill: "#8C0F2040"
    readonly property color sectorStroke: "#1a3355"
    readonly property color ringMajor: "#122030"
    readonly property color ringMinor: "#0c1826"
    readonly property color ray: "#591A3355"
    readonly property color boresight: "#1F1FD0AA"
    readonly property color sensorLabel: "#8031D0AA"

    readonly property string monoFont: "Cascadia Mono"
}
