// StatusHeader.axaml: brand on the left; timestamp, scan status and presence badge on the right.
import QtQuick
import Sarwave.Gui
import QtQuick.Layouts

Rectangle {
    id: root

    required property StatusViewModel viewModel
    readonly property bool detected: viewModel.presenceDetected

    color: Tokens.surface
    border.color: Tokens.border
    border.width: 1
    radius: 8

    // Left: brand
    RowLayout {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        Text {
            text: "SARWAVE NODE"
            color: Tokens.primary
            font.pixelSize: 12
            font.weight: Font.Bold
            font.letterSpacing: 1.2
        }
        Text {
            text: root.viewModel.deviceDescription
            color: Tokens.muted
            font.pixelSize: 10
        }
    }

    // Right: timestamp | status | presence badge
    RowLayout {
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        Text {
            text: root.viewModel.lastUpdated
            color: Tokens.muted
            font.pixelSize: 10
            font.family: Tokens.monoFont
        }

        Rectangle {
            implicitWidth: 1
            implicitHeight: 18
            color: Tokens.border
        }

        Text {
            text: root.viewModel.statusText
            color: root.detected ? Tokens.accent : Tokens.muted
            font.pixelSize: 10
            font.weight: Font.DemiBold
            font.letterSpacing: 0.5
        }

        Rectangle {
            implicitWidth: 1
            implicitHeight: 18
            color: Tokens.border
        }

        // Presence badge: red "CLEAR" by default, green when detected.
        Rectangle {
            implicitWidth: 96
            implicitHeight: 22
            radius: 11
            border.width: 1
            color: root.detected ? "#0e3329" : "#391111"
            border.color: root.detected ? "#1d5244" : "#551c1c"

            Row {
                anchors.centerIn: parent
                spacing: 6

                Rectangle {
                    id: dot
                    width: 5
                    height: 5
                    radius: 2.5
                    anchors.verticalCenter: parent.verticalCenter
                    color: root.detected ? Tokens.accent : Tokens.danger

                    // Avalonia eases the whole 1 s keyframe run (1 -> 0.2 -> 1) with SineEaseInOut.
                    property real progress: 0
                    opacity: root.detected ? (progress < 0.5 ? 1 - 1.6 * progress : 0.2 + 1.6 * (progress - 0.5)) : 1

                    NumberAnimation on progress {
                        running: root.detected
                        from: 0
                        to: 1
                        duration: 1000
                        easing.type: Easing.InOutSine
                        loops: Animation.Infinite
                    }
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.viewModel.badgeText
                    color: root.detected ? "#9be8d4" : "#ff8a8a"
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                    font.letterSpacing: 0.4
                }
            }
        }
    }
}
