// LAN address dialogue: the look of LanDialogue.axaml. Its logic is the C++ LanDialogue object
// handed in as `controller`.
import QtQuick
import Sarwave.DevEnv
import QtQuick.Controls.Universal
import QtQuick.Layouts

Window {
    id: root

    required property LanDialogue controller

    title: root.controller.title
    width: 520
    height: 360
    minimumWidth: 440
    minimumHeight: 320
    color: "#111d31"

    Universal.theme: Universal.Dark
    Universal.accent: "#0078d4"

    Connections {
        target: root.controller
        function onCloseRequested() {
            root.close()
        }
    }

    component Label13: Text {
        color: "#f4f7fb"
        font.pixelSize: 13
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
    }

    component Note: Text {
        color: "#91a4c2"
        font.pixelSize: 12
        wrapMode: Text.Wrap
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Note {
            Layout.fillWidth: true
            text: root.controller.description
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 12
            rowSpacing: 12

            Label13 {
                text: root.controller.addressLabel
            }
            TextField {
                id: addressField
                Layout.fillWidth: true
                text: root.controller.initialHost
                focus: true
                onAccepted: okButton.clicked()
            }

            Label13 {
                text: "Port:"
            }
            SpinBox {
                id: portSpin
                Layout.fillWidth: true
                from: 1
                to: 65535
                value: root.controller.initialPort
                editable: true
                // "0": no digit grouping.
                textFromValue: (value, locale) => value.toString()
                valueFromText: (text, locale) => parseInt(text)
            }
        }

        // This device's addresses, to read out for the other side; selectable to copy.
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: addressColumn.implicitHeight + 16
            color: "#0b1424"
            radius: 4

            ColumnLayout {
                id: addressColumn
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                anchors.topMargin: 8
                anchors.bottomMargin: 8
                spacing: 4

                Note {
                    text: "This device's IP address:"
                }
                TextEdit {
                    Layout.fillWidth: true
                    readOnly: true
                    selectByMouse: true
                    color: "#f4f7fb"
                    selectionColor: "#0078d4"
                    font.pixelSize: 16
                    font.family: "Cascadia Mono"
                    text: root.controller.localAddresses
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }

        Text {
            Layout.fillWidth: true
            color: "#ff6b6b"
            font.pixelSize: 12
            wrapMode: Text.Wrap
            text: root.controller.errorText
            visible: text.length > 0
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8

            Button {
                text: root.controller.cancelText
                onClicked: root.controller.cancel()
            }
            Button {
                id: okButton
                text: root.controller.okText
                highlighted: true
                onClicked: root.controller.ok(addressField.text, portSpin.value)
            }
        }
    }
}
