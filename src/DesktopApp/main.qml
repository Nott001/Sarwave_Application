import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// ──────────────────────────────────────────────────────────────────────────────
// Sarwave Desktop Dashboard — mmWave Radar Detection Monitor
// Fixes: (1) Radar scope layout compression, (2) Removed unused beam-sweep code,
//        (3) Replaced high-frequency repaint timer with signal-driven repaints only.
// ──────────────────────────────────────────────────────────────────────────────
ApplicationWindow {
    id: root
    width: 1280
    height: 800
    minimumWidth: 1024
    minimumHeight: 680
    visible: true
    title: "Sarwave | Detection Monitor"
    color: "#09111f"

    // ── Design Tokens ──────────────────────────────────────────────────────────
    readonly property color clrBackground: "#09111f"
    readonly property color clrSurface:    "#111d31"
    readonly property color clrRaised:     "#172640"
    readonly property color clrBorder:     "#1e3052"
    readonly property color clrPrimary:    "#f4f7fb"
    readonly property color clrMuted:      "#91a4c2"
    readonly property color clrAccent:     "#31d0aa"
    readonly property color clrDanger:     "#ff4757"
    readonly property color clrWarn:       "#ffa502"
    readonly property color clrBlue:       "#73a9ff"
    readonly property color clrPurple:     "#b9a5ff"
    readonly property color clrYellow:     "#feca57"
    readonly property color clrCyan:       "#48dbfb"
    readonly property color clrRed:        "#ff6b81"

    // ── Reusable MetricCard component ─────────────────────────────────────────
    component MetricCard: Rectangle {
        required property string label
        required property string value
        required property string unit
        required property color  accentColor

        Layout.fillWidth: true
        Layout.preferredHeight: 100
        color: root.clrRaised
        radius: 14
        border.color: root.clrBorder
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 0

            // Label row
            RowLayout {
                Layout.fillWidth: true
                spacing: 0

                Text {
                    text: label.toUpperCase()
                    color: root.clrMuted
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                    font.letterSpacing: 1.2
                }
                Item { Layout.fillWidth: true }
                Rectangle {
                    width: 7; height: 7; radius: 4
                    color: accentColor
                }
            }

            Item { Layout.fillHeight: true }

            // Value row
            RowLayout {
                spacing: 4

                Text {
                    text: value
                    color: root.clrPrimary
                    font.pixelSize: 26
                    font.weight: Font.DemiBold
                }
                Text {
                    text: unit
                    color: root.clrMuted
                    font.pixelSize: 12
                    Layout.alignment: Qt.AlignBottom | Qt.AlignLeft
                    bottomPadding: 3
                }
            }
        }
    }

    // ── Header / Toolbar ──────────────────────────────────────────────────────
    header: ToolBar {
        height: 72
        padding: 0
        background: Rectangle {
            color: root.clrSurface
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width; height: 1
                color: root.clrBorder
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 28
            anchors.rightMargin: 28
            spacing: 14

            // Brand icon
            Rectangle {
                width: 38; height: 38; radius: 11
                color: root.clrAccent

                Text {
                    anchors.centerIn: parent
                    text: "S"
                    color: "#06251f"
                    font.bold: true
                    font.pixelSize: 20
                    font.letterSpacing: 0.5
                }
            }

            // Brand text
            ColumnLayout {
                spacing: 1

                Text {
                    text: "SARWAVE"
                    color: root.clrPrimary
                    font.pixelSize: 16
                    font.bold: true
                    font.letterSpacing: 2.0
                }
                Text {
                    text: "mmWave Radar Detection Monitor"
                    color: root.clrMuted
                    font.pixelSize: 11
                }
            }

            Item { Layout.fillWidth: true }

            // Demo mode badge
            Rectangle {
                Layout.preferredHeight: 26
                Layout.preferredWidth: 90
                radius: 13
                color: "#1a2d4a"
                border.color: root.clrBorder
                visible: dashboard.demoMode

                Text {
                    anchors.centerIn: parent
                    text: "DEMO MODE"
                    color: root.clrMuted
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                    font.letterSpacing: 1.0
                }
            }

            // Presence status badge
            Rectangle {
                Layout.preferredHeight: 34
                Layout.preferredWidth: 122
                radius: 17
                color: dashboard.presenceDetected ? "#0e3329" : "#391111"
                border.color: dashboard.presenceDetected ? "#1d5244" : "#551c1c"
                border.width: 1

                Row {
                    anchors.centerIn: parent
                    spacing: 8

                    Rectangle {
                        width: 7; height: 7; radius: 4
                        anchors.verticalCenter: parent.verticalCenter
                        color: dashboard.presenceDetected ? root.clrAccent : root.clrDanger

                        // Pulse animation when detected
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite
                            running: dashboard.presenceDetected
                            NumberAnimation { to: 0.3; duration: 600; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 1.0; duration: 600; easing.type: Easing.InOutSine }
                        }
                    }
                    Text {
                        text: dashboard.presenceDetected ? "DETECTED" : "NO TARGET"
                        color: dashboard.presenceDetected ? "#9be8d4" : "#ff8a8a"
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                        font.letterSpacing: 0.6
                    }
                }
            }

            // Refresh button
            ToolButton {
                text: "↻"
                font.pixelSize: 20
                onClicked: dashboard.refresh()
                Accessible.name: "Refresh detections"
                ToolTip.visible: hovered
                ToolTip.text: "Refresh detection data"
                ToolTip.delay: 600
            }
        }
    }

    // ── Main Layout ───────────────────────────────────────────────────────────
    // Fix Issue 1: Use Item-based anchoring so the radar scope gets a true
    // fill of remaining width, avoiding RowLayout compression of the Canvas.
    // Left panel is fixed-width; right panel fills the remainder.
    // ──────────────────────────────────────────────────────────────────────────
    Item {
        id: mainArea
        anchors.fill: parent
        anchors.margins: 28

        // ── Left Panel (fixed width) ──────────────────────────────────────────
        Item {
            id: leftPanel
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 400

            ColumnLayout {
                anchors.fill: parent
                spacing: 18

                // Section title
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Text {
                        text: "Overview"
                        color: root.clrPrimary
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Text {
                        text: "Real-time mmWave radar detection metrics"
                        color: root.clrMuted
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }

                // Presence + Confidence card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 130
                    radius: 16
                    color: root.clrSurface
                    border.color: root.clrBorder
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 18

                        // Presence section
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 5

                            Text {
                                text: "PRESENCE"
                                color: root.clrMuted
                                font.pixelSize: 10
                                font.weight: Font.DemiBold
                                font.letterSpacing: 1.4
                            }
                            Text {
                                text: dashboard.presenceDetected ? "DETECTED" : "NONE"
                                color: dashboard.presenceDetected ? root.clrAccent : root.clrDanger
                                font.pixelSize: 22
                                font.weight: Font.DemiBold
                            }
                            Text {
                                text: dashboard.presenceDetected
                                      ? "SVM confirms occupant"
                                      : "No signature in FOV"
                                color: root.clrMuted
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }

                        // Divider
                        Rectangle {
                            Layout.preferredWidth: 1
                            Layout.fillHeight: true
                            color: root.clrBorder
                        }

                        // Confidence section
                        ColumnLayout {
                            spacing: 4

                            Text {
                                text: "CONFIDENCE"
                                color: root.clrMuted
                                font.pixelSize: 10
                                font.weight: Font.DemiBold
                                font.letterSpacing: 1.4
                            }
                            Text {
                                text: (dashboard.classificationConfidence * 100).toFixed(1) + "%"
                                color: root.clrPrimary
                                font.pixelSize: 28
                                font.weight: Font.DemiBold
                            }
                            Text {
                                text: "SVM score"
                                color: root.clrMuted
                                font.pixelSize: 10
                            }
                        }
                    }
                }

                // Metric cards grid
                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: 12
                    rowSpacing: 12

                    MetricCard {
                        label: "Range"
                        value: dashboard.distance.toFixed(2)
                        unit: "m"
                        accentColor: root.clrAccent
                    }
                    MetricCard {
                        label: "Doppler"
                        value: dashboard.dopplerVelocity.toFixed(3)
                        unit: "m/s"
                        accentColor: root.clrBlue
                    }
                    MetricCard {
                        label: "Density"
                        value: dashboard.pointDensity.toFixed(0)
                        unit: "pts"
                        accentColor: root.clrPurple
                    }
                    MetricCard {
                        label: "SNR"
                        value: dashboard.snr.toFixed(1)
                        unit: "dB"
                        accentColor: root.clrRed
                    }
                    MetricCard {
                        label: "Spread"
                        value: dashboard.spatialSpread.toFixed(2)
                        unit: "m³"
                        accentColor: root.clrYellow
                    }
                    MetricCard {
                        label: "Centroid (X,Y)"
                        value: dashboard.centroidX.toFixed(1) + ", " + dashboard.centroidY.toFixed(1)
                        unit: "m"
                        accentColor: root.clrCyan
                    }
                }

                Item { Layout.fillHeight: true }

                // Footer timestamp + connection status
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Rectangle {
                        width: 6; height: 6; radius: 3
                        color: root.clrAccent
                        Layout.alignment: Qt.AlignVCenter
                    }
                    Text {
                        text: dashboard.connectionStatus
                        color: root.clrAccent
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: "Updated " + dashboard.lastUpdated
                        color: root.clrMuted
                        font.pixelSize: 11
                    }
                }
            }
        }

        // ── Vertical Divider ──────────────────────────────────────────────────
        Rectangle {
            id: divider
            anchors.left: leftPanel.right
            anchors.leftMargin: 20
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: root.clrBorder
        }

        // ── Right Panel (fills remaining width) ───────────────────────────────
        Item {
            id: rightPanel
            anchors.left: divider.right
            anchors.leftMargin: 20
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            ColumnLayout {
                anchors.fill: parent
                spacing: 12

                // Scope header bar
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
                    radius: 14
                    color: root.clrSurface
                    border.color: root.clrBorder
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 20
                        anchors.rightMargin: 20

                        Text {
                            text: "Radar Scope"
                            color: root.clrPrimary
                            font.pixelSize: 17
                            font.weight: Font.DemiBold
                        }
                        Item { Layout.fillWidth: true }

                        // IWR6843 chip label
                        Text {
                            text: "IWR6843AOP"
                            color: root.clrMuted
                            font.pixelSize: 11
                            font.family: "monospace"
                        }

                        Rectangle {
                            Layout.leftMargin: 12
                            Layout.preferredWidth: 1
                            Layout.preferredHeight: 22
                            color: root.clrBorder
                        }

                        Text {
                            Layout.leftMargin: 12
                            text: dashboard.presenceDetected ? "TARGET ACQUIRED" : "SCANNING…"
                            color: dashboard.presenceDetected ? root.clrAccent : root.clrMuted
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            font.letterSpacing: 0.8
                        }
                    }
                }

                // ── Radar Scope Canvas container ──────────────────────────────
                // Fix Issue 1: This Rectangle now fills all remaining height and
                // width because it's inside an Item that is anchored to fill
                // rightPanel, which itself anchors to the parent's right edge.
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 16
                    color: root.clrSurface
                    border.color: root.clrBorder
                    border.width: 1
                    clip: true

                    Canvas {
                        id: radar
                        anchors.fill: parent
                        anchors.margins: 20

                        // Fix Issue 2: No beam-sweep drawing — paint is purely
                        // data-driven (range grid + FOV + azimuth + point cloud).
                        // Repaints are triggered only by data changes (signal-driven),
                        // removing the wasteful 50ms polling timer.
                        onPaint: {
                            const ctx = getContext("2d");
                            const w   = width;
                            const h   = height;

                            // Guard against zero-size canvas during init
                            if (w < 1 || h < 1) return;

                            // Layout constants
                            const labelW    = 40;          // left margin for range labels
                            const marginTop = 16;
                            const marginBot = 28;          // space below sensor for label
                            const marginR   = 16;

                            const plotLeft   = labelW;
                            const plotRight  = w - marginR;
                            const plotTop    = marginTop;
                            const plotBottom = h - marginBot;
                            const plotW      = plotRight - plotLeft;
                            const plotH      = plotBottom - plotTop;

                            const sensorX    = plotLeft + plotW / 2;
                            const sensorY    = plotBottom;

                            const maxRange      = 5.0;              // metres
                            const fovHalfRad    = Math.PI / 4;      // ±45 °

                            // Helper: range (m) → canvas Y coordinate
                            const rangeToY = (r) => plotBottom - (r / maxRange) * plotH;

                            // ── Clear ─────────────────────────────────────────
                            ctx.reset();
                            ctx.fillStyle = "#050d18";
                            ctx.fillRect(0, 0, w, h);

                            // ── Range grid lines ──────────────────────────────
                            ctx.font        = "10px monospace";
                            ctx.textAlign   = "right";
                            ctx.textBaseline = "middle";

                            for (let r = 1; r <= 5; ++r) {
                                const y = rangeToY(r);

                                ctx.beginPath();
                                ctx.moveTo(plotLeft, y);
                                ctx.lineTo(plotRight, y);
                                ctx.strokeStyle = "#0f2038";
                                ctx.lineWidth   = 0.8;
                                ctx.stroke();

                                ctx.fillStyle = "#91a4c2";
                                ctx.fillText(r + "m", plotLeft - 6, y);
                            }

                            // ── FOV sector fill ───────────────────────────────
                            const fovLeftX  = sensorX - Math.tan(fovHalfRad) * plotH;
                            const fovRightX = sensorX + Math.tan(fovHalfRad) * plotH;

                            ctx.beginPath();
                            ctx.moveTo(sensorX, sensorY);
                            ctx.lineTo(fovLeftX,  plotTop);
                            ctx.lineTo(fovRightX, plotTop);
                            ctx.closePath();
                            ctx.fillStyle   = "rgba(15, 32, 64, 0.55)";
                            ctx.fill();
                            ctx.strokeStyle = "#1a3355";
                            ctx.lineWidth   = 1.0;
                            ctx.stroke();

                            // ── Azimuth markers ───────────────────────────────
                            const aziAngles = [-45, -30, -15, 0, 15, 30, 45];
                            ctx.font        = "9px monospace";
                            ctx.textAlign   = "center";
                            ctx.textBaseline = "top";

                            for (const aDeg of aziAngles) {
                                const rad  = (aDeg * Math.PI) / 180;
                                const ax   = sensorX + Math.tan(rad) * plotH;

                                // Tick mark
                                ctx.beginPath();
                                ctx.moveTo(ax, plotBottom);
                                ctx.lineTo(ax, plotBottom + 4);
                                ctx.strokeStyle = "#263d60";
                                ctx.lineWidth   = 1;
                                ctx.stroke();

                                // Label
                                ctx.fillStyle = "#91a4c2";
                                ctx.fillText(aDeg + "°", ax, plotBottom + 6);

                                // Faint radial guide line
                                if (aDeg !== 0) {
                                    ctx.beginPath();
                                    ctx.moveTo(sensorX, sensorY);
                                    ctx.lineTo(ax, plotTop);
                                    ctx.strokeStyle = "rgba(26, 51, 85, 0.4)";
                                    ctx.lineWidth   = 0.5;
                                    ctx.stroke();
                                }
                            }

                            // ── Sensor icon (IWR6843) ─────────────────────────
                            // Outer ring
                            ctx.beginPath();
                            ctx.arc(sensorX, sensorY, 8, 0, Math.PI * 2);
                            ctx.fillStyle = "#172640";
                            ctx.fill();
                            ctx.strokeStyle = root.clrAccent;
                            ctx.lineWidth   = 1.5;
                            ctx.stroke();

                            // Inner dot
                            ctx.beginPath();
                            ctx.arc(sensorX, sensorY, 3.5, 0, Math.PI * 2);
                            ctx.fillStyle = root.clrAccent;
                            ctx.fill();

                            // ── Point cloud cluster ───────────────────────────
                            if (dashboard.presenceDetected) {
                                // Map centroid to canvas coordinates
                                // Standard radar convention: Y = forward, X = lateral.
                                // atan2(X, Y) gives the signed bearing from boresight.
                                const azimuthDeg = Math.atan2(dashboard.centroidX, dashboard.centroidY) * 180 / Math.PI;
                                const clampedAz  = Math.max(-40, Math.min(40, azimuthDeg));
                                const tRad       = (clampedAz * Math.PI) / 180;
                                const centerX    = sensorX + Math.tan(tRad) * plotH;
                                const centerY    = rangeToY(Math.max(0.5, Math.min(maxRange, dashboard.distance)));

                                // Cluster halo
                                const spreadPx  = Math.min(Math.max(dashboard.spatialSpread * 22, 8), 28);
                                const gradient  = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, spreadPx * 1.8);
                                gradient.addColorStop(0.0, "rgba(255, 71, 87, 0.18)");
                                gradient.addColorStop(1.0, "rgba(255, 71, 87, 0.0)");
                                ctx.beginPath();
                                ctx.arc(centerX, centerY, spreadPx * 1.8, 0, Math.PI * 2);
                                ctx.fillStyle = gradient;
                                ctx.fill();

                                // Individual point cloud dots (FOV-clipped)
                                const numPoints = Math.min(Math.floor(dashboard.pointDensity / 15), 20);
                                for (let i = 0; i < numPoints; ++i) {
                                    // Try up to 12 times to land inside the FOV sector
                                    let px, py, valid = false;
                                    for (let attempt = 0; attempt < 12; ++attempt) {
                                        const angle = Math.random() * Math.PI * 2;
                                        const dist  = Math.random() * spreadPx;
                                        px = centerX + Math.cos(angle) * dist;
                                        py = centerY + Math.sin(angle) * dist;

                                        // FOV half-width at height py
                                        const halfW = Math.tan(fovHalfRad) * (sensorY - py);
                                        if (px >= sensorX - halfW && px <= sensorX + halfW
                                                && py >= plotTop && py <= plotBottom) {
                                            valid = true;
                                            break;
                                        }
                                    }
                                    if (!valid) continue;

                                    const alpha = 0.45 + Math.random() * 0.5;
                                    const size  = 1.2 + Math.random() * 1.8;
                                    ctx.beginPath();
                                    ctx.arc(px, py, size, 0, Math.PI * 2);
                                    ctx.fillStyle = "rgba(255, 71, 87, " + alpha.toFixed(2) + ")";
                                    ctx.fill();
                                }

                                // Cluster annotation
                                ctx.font        = "10px monospace";
                                ctx.textAlign   = "left";
                                ctx.textBaseline = "bottom";
                                ctx.fillStyle   = "rgba(255, 71, 87, 0.85)";
                                ctx.fillText("OCCLUDED", centerX + 4, centerY - spreadPx - 4);

                                ctx.textBaseline = "top";
                                ctx.fillStyle   = "rgba(145, 164, 194, 0.9)";
                                ctx.fillText(
                                    dashboard.distance.toFixed(1) + "m · " +
                                    dashboard.dopplerVelocity.toFixed(3) + " m/s",
                                    centerX + 4,
                                    centerY + spreadPx + 2
                                );
                            }
                        }

                        // Fix Issue 2 & efficiency: removed the 50ms repaint Timer.
                        // Repaints are triggered solely when detection data changes.
                        Connections {
                            target: dashboard
                            function onDetectionsChanged() { radar.requestPaint(); }
                        }

                        Component.onCompleted: requestPaint()
                    }

                    // "No signal" overlay when canvas is too small to render
                    Text {
                        anchors.centerIn: parent
                        visible: radar.width < 80 || radar.height < 80
                        text: "Resize window to view radar scope"
                        color: root.clrMuted
                        font.pixelSize: 13
                    }
                }
            }
        }
    }
}
