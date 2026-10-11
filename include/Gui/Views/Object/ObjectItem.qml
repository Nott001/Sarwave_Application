// ObjectItem.axaml: one tracked target as a card. Raised, not Surface: it sits inside
// ObjectsPanel, which is already a Surface card. When detection is lost the card stays listed
// while the target's points fade, with all its colour drained and the whole card dimmed.
import QtQuick
import Sarwave.Gui
import QtQuick.Layouts

Rectangle {
    id: root

    required property ObjectItemViewModel viewModel
    readonly property bool dropped: !viewModel.isActive

    implicitHeight: content.implicitHeight + 20
    color: Tokens.raised
    border.color: Tokens.border
    border.width: 1
    radius: 8
    opacity: dropped ? 0.6 : 1

    component Caption: Text {
        color: Tokens.muted
        font.pixelSize: 9
        font.weight: Font.DemiBold
        font.letterSpacing: 0.9
    }

    component Rule: Rectangle {
        Layout.preferredWidth: 1
        Layout.fillHeight: true
        Layout.topMargin: 6
        Layout.bottomMargin: 6
        color: Tokens.border
    }

    RowLayout {
        id: content
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        spacing: 0

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            // Row 1: id | confidence
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                spacing: 18

                // The id takes the colour its target and points have on the radar plot.
                Text {
                    Layout.alignment: Qt.AlignVCenter
                    text: root.viewModel.idText
                    color: root.dropped ? Tokens.muted : TargetColourConverter.convert(root.viewModel.objectId)
                    font.family: Tokens.monoFont
                    font.pixelSize: 40
                    font.weight: Font.ExtraBold
                }

                Column {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 1

                    Caption {
                        text: "CONFIDENCE"
                    }
                    Text {
                        text: root.viewModel.confidenceText
                        color: root.dropped ? Tokens.muted : Tokens.accent
                        font.pixelSize: 22
                        font.weight: Font.DemiBold
                    }
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            // Row 2: the exact figures behind the bar.
            RowLayout {
                Layout.fillWidth: true
                spacing: 0

                MetricCell {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    label: "HEIGHT"
                    unit: "m"
                    value: root.viewModel.heightText
                    valueColor: root.dropped ? Tokens.muted : Tokens.purple
                }
                Rule {}
                MetricCell {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    label: "MIN Z"
                    unit: "m"
                    value: root.viewModel.minZText
                    valueColor: root.dropped ? Tokens.muted : Tokens.blue
                }
                Rule {}
                MetricCell {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    label: "MAX Z"
                    unit: "m"
                    value: root.viewModel.maxZText
                    valueColor: root.dropped ? Tokens.muted : Tokens.blue
                }
            }
        }

        // Scale labels beside the bar: top of the scale at the top, bottom at the bottom.
        RowLayout {
            Layout.fillHeight: true
            spacing: 6

            ColumnLayout {
                Layout.fillHeight: true
                spacing: 0

                Caption {
                    Layout.alignment: Qt.AlignRight
                    text: StringFormat.fixed(zBar.maximum, 0, 1) + " m"
                }
                Caption {
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    Layout.fillHeight: true
                    verticalAlignment: Text.AlignVCenter
                    text: "Z EXTENT"
                }
                Caption {
                    Layout.alignment: Qt.AlignRight
                    text: StringFormat.fixed(zBar.minimum, 0, 1) + " m"
                }
            }

            ZRangeBar {
                id: zBar
                Layout.preferredWidth: 12
                Layout.fillHeight: true
                orientation: Qt.Vertical
                low: root.viewModel.minZ
                high: root.viewModel.maxZ
                trackBrush: Tokens.plotBackground
                tickBrush: Tokens.border
                fillBrush: root.dropped ? Tokens.muted : Tokens.purple
            }
        }
    }
}
