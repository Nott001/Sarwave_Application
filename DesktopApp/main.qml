import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1280; height: 800
    minimumWidth: 960; minimumHeight: 640
    visible: true
    title: "Sarwave | Monitoring"
    color: "#09111f"

    readonly property color surface: "#111d31"
    readonly property color raised: "#172640"
    readonly property color borderColor: "#263957"
    readonly property color primary: "#f4f7fb"
    readonly property color muted: "#91a4c2"
    readonly property color accent: "#31d0aa"

    component MetricCard: Rectangle {
        required property string label
        required property string value
        required property string unit
        required property color accentColor
        Layout.fillWidth: true; Layout.preferredHeight: 148
        color: root.raised; radius: 16; border.color: root.borderColor
        ColumnLayout {
            anchors.fill: parent; anchors.margins: 20; spacing: 4
            RowLayout {
                Layout.fillWidth: true
                Text { text: label.toUpperCase(); color: root.muted; font.pixelSize: 12; font.weight: Font.DemiBold; font.letterSpacing: 1 }
                Item { Layout.fillWidth: true }
                Rectangle { width: 9; height: 9; radius: 5; color: accentColor }
            }
            Item { Layout.fillHeight: true }
            RowLayout {
                spacing: 5
                Text { text: value; color: root.primary; font.pixelSize: 34; font.weight: Font.DemiBold }
                Text { text: unit; color: root.muted; font.pixelSize: 14; Layout.alignment: Qt.AlignBottom }
            }
        }
    }

    header: ToolBar {
        height: 76; padding: 0
        background: Rectangle { color: root.surface; border.color: root.borderColor }
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 32; anchors.rightMargin: 32; spacing: 14
            Rectangle { width: 36; height: 36; radius: 11; color: root.accent
                Text { anchors.centerIn: parent; text: "S"; color: "#06251f"; font.bold: true; font.pixelSize: 20 }
            }
            ColumnLayout { spacing: 1
                Text { text: "SARWAVE"; color: root.primary; font.pixelSize: 17; font.bold: true; font.letterSpacing: 1.6 }
                Text { text: "Environmental monitoring"; color: root.muted; font.pixelSize: 11 }
            }
            Item { Layout.fillWidth: true }
            Rectangle { Layout.preferredHeight: 34; Layout.preferredWidth: 112; radius: 17; color: "#12392f"; border.color: "#205b4b"
                Row { anchors.centerIn: parent; spacing: 8
                    Rectangle { width: 7; height: 7; radius: 4; color: root.accent; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: dashboard.connectionStatus; color: "#a8eedb"; font.pixelSize: 12; font.weight: Font.DemiBold }
                }
            }
            ToolButton { text: "↻"; font.pixelSize: 22; onClicked: dashboard.refresh(); Accessible.name: "Refresh readings" }
        }
    }

    ScrollView {
        anchors.fill: parent; clip: true; contentWidth: availableWidth
        ColumnLayout {
            x: 32; y: 32
            width: parent.width - 64
            spacing: 24
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout { spacing: 5
                    Text { text: "Overview"; color: root.primary; font.pixelSize: 30; font.weight: Font.DemiBold }
                    Text { text: "Live readings from the sensor node"; color: root.muted; font.pixelSize: 15 }
                }
                Item { Layout.fillWidth: true }
                Text { text: "Updated " + dashboard.lastUpdated; color: root.muted; font.pixelSize: 13 }
            }
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 190; radius: 18; color: root.surface; border.color: root.borderColor
                RowLayout {
                    anchors.fill: parent; anchors.margins: 28; spacing: 28
                    ColumnLayout { Layout.fillWidth: true; spacing: 8
                        Text { text: "CURRENT TEMPERATURE"; color: root.muted; font.pixelSize: 12; font.weight: Font.DemiBold; font.letterSpacing: 1.2 }
                        RowLayout { spacing: 8
                            Text { text: Number(dashboard.temperature).toFixed(1); color: root.primary; font.pixelSize: 68; font.weight: Font.DemiBold }
                            Text { text: "°C"; color: root.accent; font.pixelSize: 23; Layout.alignment: Qt.AlignBottom }
                        }
                        Text { text: "Within your configured comfort range"; color: "#9fe7d4"; font.pixelSize: 14 }
                    }
                    Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: root.borderColor }
                    ColumnLayout { Layout.preferredWidth: 250; spacing: 11
                        Text { text: "SYSTEM STATUS"; color: root.muted; font.pixelSize: 12; font.weight: Font.DemiBold; font.letterSpacing: 1.2 }
                        Text { text: "All sensors reporting"; color: root.primary; font.pixelSize: 18; font.weight: Font.DemiBold }
                        Text { text: "Showing simulated readings"; color: root.muted; font.pixelSize: 13; wrapMode: Text.WordWrap }
                    }
                }
            }
            GridLayout {
                Layout.fillWidth: true; columns: width >= 1050 ? 3 : 2; columnSpacing: 16; rowSpacing: 16
                MetricCard { label: "Temperature"; value: Number(dashboard.temperature).toFixed(1); unit: "°C"; accentColor: root.accent }
                MetricCard { label: "Humidity"; value: Number(dashboard.humidity).toFixed(1); unit: "% RH"; accentColor: "#73a9ff" }
                MetricCard { label: "Air pressure"; value: Number(dashboard.pressure).toFixed(1); unit: "hPa"; accentColor: "#b9a5ff" }
            }
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 230; radius: 18; color: root.surface; border.color: root.borderColor
                ColumnLayout { anchors.fill: parent; anchors.margins: 24; spacing: 12
                    RowLayout { Layout.fillWidth: true
                        Text { text: "Temperature trend"; color: root.primary; font.pixelSize: 18; font.weight: Font.DemiBold }
                        Item { Layout.fillWidth: true }
                        Text { text: "Last 30 minutes"; color: root.muted; font.pixelSize: 13 }
                    }
                    Canvas {
                        id: chart; Layout.fillWidth: true; Layout.fillHeight: true
                        onPaint: {
                            const ctx = getContext("2d"); ctx.reset(); ctx.strokeStyle = root.borderColor; ctx.lineWidth = 1
                            for (let line = 1; line < 4; ++line) { const y = height * line / 4; ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(width, y); ctx.stroke() }
                            ctx.strokeStyle = root.accent; ctx.lineWidth = 3; ctx.lineJoin = "round"; ctx.beginPath()
                            for (let x = 0; x <= width; x += 8) { const y = height * 0.52 - Math.sin((x / width) * 8 + dashboard.temperature) * 19; x === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y) }
                            ctx.stroke()
                        }
                        Connections { target: dashboard; function onReadingsChanged() { chart.requestPaint() } }
                        Component.onCompleted: requestPaint()
                    }
                }
            }
        }
    }
}
