// Configuration-file dialogue: the look of CfgDialogue.axaml. Its logic is the C++ CfgDialogue
// object handed in as `controller`.
import QtQuick
import Sarwave.DevEnv
import QtQuick.Controls.Universal
import QtQuick.Dialogs
import QtQuick.Layouts

Window {
    id: root

    required property CfgDialogue controller

    title: "MMWave Configuration File"
    width: 620
    height: 440
    minimumWidth: 520
    minimumHeight: 380
    color: "#111d31"

    Universal.theme: Universal.Dark
    Universal.accent: "#0078d4"

    onClosing: root.controller.windowClosed()

    Connections {
        target: root.controller
        function onCloseRequested() {
            root.close()
        }
        function onOutputTextChanged() {
            // Keep the newest command/response in view.
            Qt.callLater(() => outputArea.cursorPosition = outputArea.length)
        }
    }

    function scrollPathToEnd() {
        pathField.cursorPosition = pathField.text.length
    }

    Component.onCompleted: Qt.callLater(scrollPathToEnd)

    FileDialog {
        id: picker
        title: "Select Configuration File"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Configuration files (*.cfg)"]
        onAccepted: {
            pathField.text = root.controller.localPath(selectedFile)
            root.scrollPathToEnd()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Text {
            text: "Path to the Configuration"
            color: "#f4f7fb"
            font.pixelSize: 13
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: pathField
                Layout.fillWidth: true
                placeholderText: "*.cfg"
                text: root.controller.initialPath
                onActiveFocusChanged: if (!activeFocus) root.scrollPathToEnd()
            }
            Button {
                text: "Browse..."
                enabled: !root.controller.busy
                onClicked: {
                    const start = root.controller.browseStartFolder(pathField.text)
                    if (start.toString().length > 0)
                        picker.currentFolder = start
                    picker.open()
                }
            }
        }

        // Output pane: streamed command/response log.
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 140
            color: "black"

            ScrollView {
                anchors.fill: parent
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                TextArea {
                    id: outputArea
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
                text: "Apply Config"
                highlighted: true
                enabled: !root.controller.busy
                onClicked: root.controller.apply(pathField.text)
            }
            Button {
                text: "Next"
                visible: root.controller.nextVisible
                onClicked: root.controller.next()
            }
        }
    }
}
