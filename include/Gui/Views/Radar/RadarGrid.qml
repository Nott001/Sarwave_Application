// RadarGrid.axaml: the card around the scope. The grid, the data drawn over it and the sensor
// icon share one geometry, so they always line up.
import QtQuick
import Sarwave.Gui

Rectangle {
    id: root

    required property RadarGridViewModel viewModel

    color: Tokens.surface
    border.color: Tokens.border
    border.width: 1
    radius: 10
    clip: true

    Item {
        anchors.fill: parent
        anchors.margins: 10

        // Grid area (inside the label margin) + unit markers (in the margin strip around it).
        RadarGridPlot {
            id: plot
            anchors.fill: parent
            minRange: root.viewModel.minRange
            maxRange: root.viewModel.maxRange
            rangeStep: root.viewModel.rangeStep
            minAzimuth: root.viewModel.minAzimuth
            maxAzimuth: root.viewModel.maxAzimuth
            azimuthStep: root.viewModel.azimuthStep
            labelMarginLeft: 34
            labelMarginTop: 20
            labelMarginRight: 34
            labelMarginBottom: 28
            labelFontSize: 9
            labelFontFamily: Tokens.monoFont
            labelForeground: Tokens.muted
            plotBackground: Tokens.plotBackground
            sectorFill: Tokens.sectorFill
            sectorStroke: Tokens.sectorStroke
            ringMajor: Tokens.ringMajor
            ringMinor: Tokens.ringMinor
            ray: Tokens.ray
            boresight: Tokens.boresight
        }

        // Data layer: point cloud + tracked targets.
        RadarDataOverlay {
            anchors.fill: parent
            geometry: plot.geometry
            snapshot: root.viewModel.snapshot
            labelFontFamily: Tokens.monoFont
            droppedTargetBrush: Tokens.muted
        }

        // The sensor icon, glued to the apex.
        SensorMarker {
            visible: !plot.geometry.isEmpty
            x: plot.geometry.origin.x - anchorPoint.x
            y: plot.geometry.origin.y - anchorPoint.y
            label: root.viewModel.sensorLabel
        }
    }
}
