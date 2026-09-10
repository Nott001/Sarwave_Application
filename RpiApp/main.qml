import QtQuick
import QtQuick.Controls

Window {
    width: 800
    height: 480
    visible: true
    title: "Sarwave RPi Sensor Node"

    Rectangle {
        anchors.fill: parent
        color: "#1e1e1e"

        Text {
            text: "Sensor Node Initializing..."
            color: "#ffffff"
            font.pointSize: 18
            anchors.centerIn: parent
        }
    }
}
