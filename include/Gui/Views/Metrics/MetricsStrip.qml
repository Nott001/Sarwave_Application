// MetricsStrip.axaml: one combined card, four equal cells separated by thin rules.
// (Ported for completeness; MinimalGui has it commented out, as in the C# version.)
import QtQuick
import Sarwave.Gui
import QtQuick.Layouts

Rectangle {
    id: root

    required property MetricsViewModel viewModel

    color: Tokens.surface
    border.color: Tokens.border
    border.width: 1
    radius: 8

    component Rule: Rectangle {
        Layout.preferredWidth: 1
        Layout.fillHeight: true
        Layout.topMargin: 10
        Layout.bottomMargin: 10
        color: Tokens.border
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        MetricCell {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            label: "RANGE"
            unit: "m"
            value: StringFormat.fixed(root.viewModel.range, 2, 2)
            valueColor: Tokens.accent
        }
        Rule {}
        MetricCell {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            label: "DOPPLER"
            unit: "m/s"
            value: StringFormat.fixed(root.viewModel.doppler, 3, 3)
            valueColor: Tokens.blue
        }
        Rule {}
        MetricCell {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            label: "DENSITY"
            unit: "pts"
            value: StringFormat.fixed(root.viewModel.density, 0, 0)
            valueColor: Tokens.purple
        }
        Rule {}
        MetricCell {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            label: "SNR"
            unit: "dB"
            value: StringFormat.fixed(root.viewModel.snr, 1, 1)
            valueColor: Tokens.red
        }
    }
}
