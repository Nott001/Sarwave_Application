// MinimalGui.axaml: status bar over the radar grid.
import QtQuick
import Sarwave.Gui
import QtQuick.Layouts

Rectangle {
    id: root

    required property MinimalGuiViewModel viewModel

    color: Tokens.background

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        StatusHeader {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            viewModel: root.viewModel.status
        }
        RadarGrid {
            Layout.fillWidth: true
            Layout.fillHeight: true
            viewModel: root.viewModel.radar
        }
        // MetricsStrip { viewModel: root.viewModel.metrics }   (commented out in the C# version)
    }
}
