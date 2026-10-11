// MetricCell.axaml: one label / value / unit cell (e.g. RANGE 0.00 m). Pure presentation.
import QtQuick
import Sarwave.Gui

Item {
    id: root

    property string label
    property string value
    property string unit
    property color valueColor: Tokens.primary

    implicitWidth: column.implicitWidth
    implicitHeight: column.implicitHeight

    Column {
        id: column
        anchors.centerIn: parent
        spacing: 2

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.label
            color: Tokens.muted
            font.pixelSize: 9
            font.weight: Font.DemiBold
            font.letterSpacing: 0.9
        }
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 3

            Text {
                id: valueText
                text: root.value
                color: root.valueColor
                font.pixelSize: 17
                font.weight: Font.DemiBold
            }
            Text {
                // VerticalAlignment="Bottom" Margin="0,0,0,2"
                y: valueText.height - height - 2
                text: root.unit
                color: Tokens.muted
                font.pixelSize: 9
            }
        }
    }
}
