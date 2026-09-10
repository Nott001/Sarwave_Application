import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 800; height: 480
    minimumWidth: 640; minimumHeight: 400
    visible: true
    title: "Sarwave Sensor Node"
    color: "#08121f"

    readonly property color surface: "#111e31"
    readonly property color borderColor: "#263a59"
    readonly property color primary: "#f4f7fb"
    readonly property color muted: "#9aadca"
    readonly property color accent: "#31d0aa"

    component ReadingTile: Rectangle {
        required property string label
        required property string value
        required property string unit
        required property color valueColor
        Layout.fillWidth: true; Layout.fillHeight: true
        radius: 14; color: root.surface; border.color: root.borderColor
        ColumnLayout {
            anchors.fill: parent; anchors.margins: 17; spacing: 2
            Text { text: label; color: root.muted; font.pixelSize: 12; font.bold: true; font.letterSpacing: 0.8 }
            Item { Layout.fillHeight: true }
            RowLayout { spacing: 4
                Text { text: value; color: valueColor; font.pixelSize: 31; font.weight: Font.DemiBold }
                Text { text: unit; color: root.muted; font.pixelSize: 13; Layout.alignment: Qt.AlignBottom }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 24; spacing: 16
        RowLayout {
            Layout.fillWidth: true
            Rectangle { width: 34; height: 34; radius: 10; color: root.accent
                Text { anchors.centerIn: parent; text: "S"; color: "#06251f"; font.bold: true; font.pixelSize: 19 }
            }
            ColumnLayout { spacing: 0
                Text { text: "SARWAVE NODE"; color: root.primary; font.pixelSize: 16; font.bold: true; font.letterSpacing: 1.1 }
                Text { text: "Environmental sensor"; color: root.muted; font.pixelSize: 11 }
            }
            Item { Layout.fillWidth: true }
            Rectangle { width: 110; height: 30; radius: 15; color: "#123a30"; border.color: "#22604e"
                Row { anchors.centerIn: parent; spacing: 7
                    Rectangle { width: 7; height: 7; radius: 4; color: root.accent; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "READY"; color: "#aaeeda"; font.pixelSize: 11; font.bold: true }
                }
            }
        }
        Rectangle { Layout.fillWidth: true; height: 1; color: root.borderColor }
        ColumnLayout {
            Layout.fillWidth: true; spacing: 2
            Text { text: "TEMPERATURE"; color: root.muted; font.pixelSize: 12; font.bold: true; font.letterSpacing: 1.1 }
            RowLayout {
                Text { text: Number(dashboard.temperature).toFixed(1); color: root.primary; font.pixelSize: 78; font.weight: Font.DemiBold }
                Text { text: "°C"; color: root.accent; font.pixelSize: 26; Layout.alignment: Qt.AlignBottom }
            }
        }
        GridLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; columns: 2; columnSpacing: 12; rowSpacing: 12
            ReadingTile { label: "HUMIDITY"; value: Number(dashboard.humidity).toFixed(1); unit: "% RH"; valueColor: "#7fb4ff" }
            ReadingTile { label: "PRESSURE"; value: Number(dashboard.pressure).toFixed(1); unit: "hPa"; valueColor: "#c6b5ff" }
        }
        RowLayout {
            Layout.fillWidth: true
            Text { text: "Last update " + dashboard.lastUpdated; color: root.muted; font.pixelSize: 12 }
            Item { Layout.fillWidth: true }
            Button { text: "Refresh"; onClicked: dashboard.refresh(); Accessible.name: "Refresh readings" }
        }
    }
}
