// Port-selection dialogue: the look of PortDialogue.axaml. Its logic is the C++ PortDialogue
// object handed in as `controller`.
import QtQuick
import Sarwave.DevEnv
import QtQuick.Controls.Universal
import QtQuick.Layouts

Window {
    id: root

    required property PortDialogue controller

    title: "MMWave Serial Port Selection"
    width: 620
    height: 460
    minimumWidth: 520
    minimumHeight: 400
    color: "#111d31"

    Universal.theme: Universal.Dark
    Universal.accent: "#0078d4"

    onClosing: root.controller.windowClosed()

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

    component Prefix: Text {
        color: "#91a4c2"
        font.pixelSize: 13
        verticalAlignment: Text.AlignVCenter
        Layout.alignment: Qt.AlignVCenter
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Row 0: common name + separate toggle
        RowLayout {
            spacing: 12

            RowLayout {
                id: commonNamePanel
                spacing: 12
                visible: !separateCheck.checked

                Label13 {
                    text: "Common Name:"
                }
                TextField {
                    id: commonField
                    Layout.minimumWidth: 200
                    text: root.controller.defaultPortDirectory
                }
            }

            CheckBox {
                id: separateCheck
                text: "Use Separate Names"
            }
        }

        // Rows 1-2: CLI / Data port selectors
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 12
            rowSpacing: 12

            Label13 {
                text: "CLI Enhanced COM Port:"
            }
            Item {
                Layout.fillWidth: true
                implicitHeight: Math.max(cliNumberPanel.implicitHeight, cliNameField.implicitHeight)

                RowLayout {
                    id: cliNumberPanel
                    anchors.fill: parent
                    spacing: 1
                    visible: !separateCheck.checked

                    Prefix {
                        text: commonField.text
                    }
                    SpinBox {
                        id: cliSpin
                        Layout.fillWidth: true
                        from: 0
                        to: 999
                        value: 0
                        editable: true
                    }
                }
                TextField {
                    id: cliNameField
                    anchors.fill: parent
                    visible: separateCheck.checked
                }
            }

            Label13 {
                text: "Data Standard COM Port:"
            }
            Item {
                Layout.fillWidth: true
                implicitHeight: Math.max(dataNumberPanel.implicitHeight, dataNameField.implicitHeight)

                RowLayout {
                    id: dataNumberPanel
                    anchors.fill: parent
                    spacing: 1
                    visible: !separateCheck.checked

                    Prefix {
                        text: commonField.text
                    }
                    SpinBox {
                        id: dataSpin
                        Layout.fillWidth: true
                        from: 0
                        to: 999
                        value: 1
                        editable: true
                    }
                }
                TextField {
                    id: dataNameField
                    anchors.fill: parent
                    visible: separateCheck.checked
                }
            }
        }

        // Output pane: device version.
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 140
            color: "black"

            ScrollView {
                anchors.fill: parent
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                TextArea {
                    readOnly: true
                    selectByMouse: true
                    wrapMode: TextEdit.Wrap
                    padding: 8
                    color: "white"
                    font.family: "Cascadia Mono"
                    text: root.controller.outputText
                    background: null
                }
            }
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
                text: "Cancel"
                onClicked: root.controller.cancel()
            }
            Button {
                text: "Connect"
                highlighted: true
                enabled: !root.controller.busy
                onClicked: root.controller.connectPorts(separateCheck.checked, commonField.text, cliSpin.value,
                                                        dataSpin.value, cliNameField.text, dataNameField.text)
            }
            Button {
                text: "Next"
                visible: root.controller.nextVisible
                onClicked: root.controller.next()
            }
        }
    }
}
