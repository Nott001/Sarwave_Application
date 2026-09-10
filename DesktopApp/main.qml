import QtQuick
import QtQuick.Controls

Window {
    width: 800
    height: 480
    visible: true
    title: "Sarwave Desktop"

    Rectangle {
        anchors.fill: parent
        color: "#1e1e1e"

        Text {
            text: "Desktop App - Connecting..."
            color: "#ffffff"
            font.pointSize: 18
            anchors.centerIn: parent
        }
    }
}
