import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root

    title: "MMWave Serial Port Selection"
    modality: Qt.ApplicationModal
    width: 620
    height: 460
    minimumWidth: 520
    minimumHeight: 400
    color: "#111d31"

    property string commonName: ""
    property bool separate: false
    property int cliNumber: 0
    property int dataNumber: 0
    property string cliName: ""
    property string dataName: ""
    property bool didAccept: false
    property string errorMessage: ""
    property string outputText: ""

    FontLoader {
        id: monoFont
        source: "qrc:/QtCommon/MMWavePrompt/fonts/CascadiaMono.ttf"
    }

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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            // Row 0: Common Name
            RowLayout {
                Text {
                    visible: !root.separate
                    text: "Common Name:"
                    color: "#f4f7fb"
                    font.pixelSize: 13
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                }

                TextField {
                    visible: !root.separate
                    id: commonField
                    Layout.fillWidth: true
                    text: root.commonName
                    color: "#f4f7fb"
                    onTextChanged: root.commonName = text
                }
            }

            CheckBox {
                id: separateCheck
                text: "Use Separate Names"
                checked: root.separate
                onToggled: root.separate = checked
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 12
            rowSpacing: 12

            // Row 1: CLI Enhanced COM Port
            Text {
                text: "CLI Enhanced COM Port:"
                color: "#f4f7fb"
                font.pixelSize: 13
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }

            RowLayout {
                spacing: 1

                Text {
                    text: root.commonName
                    color: "#91a4c2"
                    font.pixelSize: 13
                    visible: !root.separate
                }

                SpinBox {
                    id: cliSpin
                    from: 0
                    to: 999
                    value: root.cliNumber
                    visible: !root.separate
                    Layout.fillWidth: true
                    onValueChanged: root.cliNumber = value
                    contentItem: TextInput {
                        text: cliSpin.displayText
                        font: cliSpin.font
                        color: "#f4f7fb"
                        horizontalAlignment: Text.AlignLeft
                        clip: true
                        validator: cliSpin.validator
                    }
                }

                TextField {
                    id: cliNameField
                    text: root.cliName
                    visible: root.separate
                    Layout.fillWidth: true
                    color: "#f4f7fb"
                    onTextChanged: root.cliName = text
                }
            }

            // Row 2: Data Standard COM Port
            Text {
                text: "Data Standard COM Port:"
                color: "#f4f7fb"
                font.pixelSize: 13
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }

            RowLayout {
                spacing: 1

                Text {
                    text: root.commonName
                    color: "#91a4c2"
                    font.pixelSize: 13
                    visible: !root.separate
                }

                SpinBox {
                    id: dataSpin
                    from: 0
                    to: 999
                    value: root.dataNumber
                    visible: !root.separate
                    Layout.fillWidth: true
                    onValueChanged: root.dataNumber = value
                    contentItem: TextInput {
                        text: dataSpin.displayText
                        font: dataSpin.font
                        color: "#f4f7fb"
                        horizontalAlignment: Text.AlignLeft
                        clip: true
                        validator: dataSpin.validator
                    }
                }

                TextField {
                    id: dataNameField
                    text: root.dataName
                    visible: root.separate
                    Layout.fillWidth: true
                    color: "#f4f7fb"
                    onTextChanged: root.dataName = text
                }
            }
        }

        ScrollView {
            id: outputView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 140
            background: Rectangle {
                color: "black"
            }

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
            visible: root.errorMessage !== ""
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
                text: "Connect"
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
