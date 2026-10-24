// ImuStrip.axaml: pitch, roll, turning rate, the IMU axis pointing up and the gate's verdict.
import QtQuick
import Sarwave.Gui
import QtQuick.Layouts

Rectangle {
    id: root

    required property ImuStatusViewModel viewModel

    color: Tokens.surface
    border.color: Tokens.border
    border.width: 1
    radius: 8

    component Label: Text {
        color: Tokens.muted
        font.pixelSize: 9
        font.weight: Font.DemiBold
        font.letterSpacing: 0.9
    }
    component Value: Text {
        color: Tokens.primary
        font.pixelSize: 11
        font.family: Tokens.monoFont
    }
    component Separator: Rectangle {
        implicitWidth: 1
        implicitHeight: 14
        color: Tokens.border
    }

    RowLayout {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        Label {
            text: "IMU"
            color: Tokens.primary
        }
        Separator {}

        Label { text: "PITCH" }
        Value {
            text: root.viewModel.pitch
            Layout.minimumWidth: 44
        }
        Label { text: "ROLL" }
        Value {
            text: root.viewModel.roll
            Layout.minimumWidth: 44
        }
        Separator {}

        Label { text: "TURNING" }
        Value {
            text: root.viewModel.turning
            Layout.minimumWidth: 58
        }
        Separator {}

        Label { text: "UP AXIS" }
        Value {
            text: root.viewModel.upAxis
            Layout.minimumWidth: 20
        }
        Separator {}

        Text {
            text: root.viewModel.gate
            color: root.viewModel.isSteady ? Tokens.accent
                 : root.viewModel.isMoving ? Tokens.danger : Tokens.muted
            font.pixelSize: 10
            font.weight: Font.DemiBold
            font.letterSpacing: 0.5
        }
    }
}
