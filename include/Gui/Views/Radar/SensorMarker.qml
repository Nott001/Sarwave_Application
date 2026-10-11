// SensorMarker.axaml: the sensor icon. The ring centre sits at (width / 2, 7): that point is
// placed on the sensor apex by RadarGrid.
import QtQuick
import Sarwave.Gui

Item {
    id: root

    property string label: "IWR6843"
    /// The point inside this item (ring centre) that sits on the sensor position.
    readonly property point anchorPoint: Qt.point(width / 2, 7)

    width: 64
    height: 30

    Rectangle {
        x: (root.width - width) / 2
        y: 0
        width: 14
        height: 14
        radius: 7
        color: Tokens.raised
        border.color: Tokens.accent
        border.width: 1.5
    }
    Rectangle {
        x: (root.width - width) / 2
        y: 4.2
        width: 5.6
        height: 5.6
        radius: 2.8
        color: Tokens.accent
    }
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 17
        text: root.label
        color: Tokens.sensorLabel
        font.family: Tokens.monoFont
        font.pixelSize: 8
    }
}
