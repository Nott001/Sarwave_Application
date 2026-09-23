import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

Window {
    id: root

    title: "MMWave Configuration File"
    modality: Qt.ApplicationModal
    width: 620
    height: 440
    minimumWidth: 520
    minimumHeight: 380
    color: "#111d31"

    property string cfgPath: ""
    property bool didAccept: false
    property string errorMessage: ""
    property string outputText: ""

    FontLoader { id: monoFont; source: "qrc:/QtCommon/MMWavePrompt/fonts/CascadiaMono.ttf" }

    signal acceptedByUser()
    signal rejectedByUser()
    signal okRequested()

    Connections {
        target: root
        function onClosing() {
            if (root.didAccept) {
                root.acceptedByUser()
            } else {
                root.rejectedByUser()
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select Configuration File"
        nameFilters: ["Configuration files (*.cfg)"]
        onAccepted: root.cfgPath = fileDialog.selectedFile
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
                text: root.cfgPath
                color: "#f4f7fb"
                placeholderText: "*.cfg"
                onTextChanged: root.cfgPath = text
            }

            Button {
                text: "Browse..."
                onClicked: fileDialog.open()
            }
        }

        ScrollView {
            id: outputView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 140
            background: Rectangle { color: "black" }

            TextArea {
                width: outputView.width
                text: root.outputText
                color: "white"
                font.family: monoFont.name
                readOnly: true
                selectByMouse: true
                selectByKeyboard: true
                selectionColor: "black"
                selectedTextColor: "white"
                wrapMode: Text.Wrap
                implicitHeight: contentHeight
                padding: 8
            }
        }

        Text {
            text: root.errorMessage
            color: "#ff6b6b"
            font.pixelSize: 12
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            visible: root.errorMessage != ""
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Item { Layout.fillWidth: true }

            Button {
                text: "Cancel"
                onClicked: root.close()
            }

            Button {
                text: "Apply Config"
                highlighted: true
                onClicked: root.okRequested()
            }

            Button {
                text: "Next"
                visible: root.didAccept
                enabled: root.didAccept
                onClicked: root.close()
            }
        }
    }
}
