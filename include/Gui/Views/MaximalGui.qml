// MaximalGui.axaml: status bar and IMU strip over the radar grid, with the objects panel (300
// wide) beside.
import QtQuick
import Sarwave.Gui
import QtQuick.Layouts

Rectangle {
    id: root

    required property MaximalGuiViewModel viewModel

    color: Tokens.background

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            StatusHeader {
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                viewModel: root.viewModel.status
            }
            // Only in this desktop dashboard (wireless and recorded apps), not in the device's own
            // MinimalGui. Hidden while the session has no IMU.
            ImuStrip {
                Layout.fillWidth: true
                Layout.preferredHeight: 28
                visible: root.viewModel.imu.isAvailable
                viewModel: root.viewModel.imu
            }
            RadarGrid {
                Layout.fillWidth: true
                Layout.fillHeight: true
                viewModel: root.viewModel.radar
            }
        }

        ObjectsPanel {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            viewModel: root.viewModel.objects
        }
    }
}
