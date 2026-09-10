import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// ──────────────────────────────────────────────────────────────────────────────
// Sarwave RPi Node UI — small LCD optimised layout (800×480)
//
// Stack:
//   [header bar ~36px]
//   [radar scope — fills all available height]
//   [unified metrics strip ~52px]
// ──────────────────────────────────────────────────────────────────────────────
ApplicationWindow {
    id: root
    width: 800; height: 480
    minimumWidth: 640; minimumHeight: 400
    visible: true
    title: "Sarwave Sensor Node"
    color: "#09111f"

    // ── Design tokens ──────────────────────────────────────────────────────────
    readonly property color clrSurface:  "#111d31"
    readonly property color clrRaised:   "#172640"
    readonly property color clrBorder:   "#1e3052"
    readonly property color clrPrimary:  "#f4f7fb"
    readonly property color clrMuted:    "#91a4c2"
    readonly property color clrAccent:   "#31d0aa"
    readonly property color clrDanger:   "#ff4757"
    readonly property color clrBlue:     "#73a9ff"
    readonly property color clrPurple:   "#b9a5ff"
    readonly property color clrRed:      "#ff6b81"

    // ── Root vertical stack ────────────────────────────────────────────────────
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        // ── HEADER BAR ────────────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: root.clrSurface
            radius: 8
            border.color: root.clrBorder
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 10

                // Brand icon
                Rectangle {
                    width: 22; height: 22; radius: 6
                    color: root.clrAccent
                    Text {
                        anchors.centerIn: parent
                        text: "S"; color: "#06251f"
                        font.bold: true; font.pixelSize: 13
                    }
                }

                // Brand text
                Text {
                    text: "SARWAVE NODE"
                    color: root.clrPrimary
                    font.pixelSize: 12
                    font.bold: true
                    font.letterSpacing: 1.2
                }

                Text {
                    text: "IWR6843AOP · mmWave Radar"
                    color: root.clrMuted
                    font.pixelSize: 10
                }

                Item { Layout.fillWidth: true }

                // Timestamp
                Text {
                    text: dashboard.lastUpdated
                    color: root.clrMuted
                    font.pixelSize: 10
                    font.family: "monospace"
                }

                // Vertical rule
                Rectangle {
                    width: 1; height: 18
                    color: root.clrBorder
                }

                // Scan status
                Text {
                    text: dashboard.presenceDetected ? "TARGET ACQUIRED" : "SCANNING…"
                    color: dashboard.presenceDetected ? root.clrAccent : root.clrMuted
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                    font.letterSpacing: 0.5
                }

                // Vertical rule
                Rectangle {
                    width: 1; height: 18
                    color: root.clrBorder
                }

                // Presence badge — the only presence indicator
                Rectangle {
                    Layout.preferredHeight: 22
                    Layout.preferredWidth: 96
                    radius: 11
                    color:  dashboard.presenceDetected ? "#0e3329" : "#391111"
                    border.color: dashboard.presenceDetected ? "#1d5244" : "#551c1c"
                    border.width: 1

                    Row {
                        anchors.centerIn: parent
                        spacing: 6

                        Rectangle {
                            width: 5; height: 5; radius: 3
                            anchors.verticalCenter: parent.verticalCenter
                            color: dashboard.presenceDetected ? root.clrAccent : root.clrDanger

                            SequentialAnimation on opacity {
                                loops: Animation.Infinite
                                running: dashboard.presenceDetected
                                NumberAnimation { to: 0.2; duration: 500; easing.type: Easing.InOutSine }
                                NumberAnimation { to: 1.0; duration: 500; easing.type: Easing.InOutSine }
                            }
                        }
                        Text {
                            text: dashboard.presenceDetected ? "DETECTED" : "CLEAR"
                            color: dashboard.presenceDetected ? "#9be8d4" : "#ff8a8a"
                            font.pixelSize: 10
                            font.weight: Font.DemiBold
                            font.letterSpacing: 0.4
                        }
                    }
                }
            }
        }

        // ── RADAR SCOPE — hero element ─────────────────────────────────────────
        Rectangle {
            Layout.fillWidth:  true
            Layout.fillHeight: true   // consumes all remaining vertical space
            color: root.clrSurface
            radius: 10
            border.color: root.clrBorder
            border.width: 1
            clip: true

            Canvas {
                id: radar
                anchors.fill: parent
                anchors.margins: 10

                onPaint: {
                    const ctx = getContext("2d");
                    const w   = width;
                    const h   = height;
                    if (w < 1 || h < 1) return;

                    // ── Layout geometry ───────────────────────────────────────
                    const labelW    = 28;
                    const marginTop = 10;
                    const marginBot = 20;   // room for azimuth labels
                    const marginR   = 8;

                    const plotLeft   = labelW;
                    const plotRight  = w - marginR;
                    const plotTop    = marginTop;
                    const plotBottom = h - marginBot;
                    const plotW      = plotRight - plotLeft;
                    const plotH      = plotBottom - plotTop;

                    const sensorX    = plotLeft + plotW / 2;
                    const sensorY    = plotBottom;

                    const maxRange   = 5.0;
                    const fovHalfRad = Math.PI / 4;  // ±45°

                    const rangeToY = (r) => plotBottom - (r / maxRange) * plotH;

                    // ── Clear ─────────────────────────────────────────────────
                    ctx.reset();
                    ctx.fillStyle = "#050d18";
                    ctx.fillRect(0, 0, w, h);

                    // ── Range rings ───────────────────────────────────────────
                    ctx.font         = "9px monospace";
                    ctx.textAlign    = "right";
                    ctx.textBaseline = "middle";

                    for (let r = 1; r <= 5; ++r) {
                        const y = rangeToY(r);
                        ctx.beginPath();
                        ctx.moveTo(plotLeft, y);
                        ctx.lineTo(plotRight, y);
                        ctx.strokeStyle = r % 2 === 0 ? "#122030" : "#0c1826";
                        ctx.lineWidth   = r % 2 === 0 ? 0.9 : 0.5;
                        ctx.stroke();
                        ctx.fillStyle = "#91a4c2";
                        ctx.fillText(r + "m", plotLeft - 3, y);
                    }

                    // ── FOV sector ────────────────────────────────────────────
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
                    ctx.lineWidth   = 0.9;
                    ctx.stroke();

                    // ── Azimuth markers ───────────────────────────────────────
                    const aziAngles = [-45, -30, -15, 0, 15, 30, 45];
                    ctx.font         = "8px monospace";
                    ctx.textAlign    = "center";
                    ctx.textBaseline = "top";

                    for (const aDeg of aziAngles) {
                        const rad = (aDeg * Math.PI) / 180;
                        const ax  = sensorX + Math.tan(rad) * plotH;

                        // Radial guide line (except boresight — drawn separately)
                        if (aDeg !== 0) {
                            ctx.beginPath();
                            ctx.moveTo(sensorX, sensorY);
                            ctx.lineTo(ax, plotTop);
                            ctx.strokeStyle = "rgba(26, 51, 85, 0.35)";
                            ctx.lineWidth   = 0.5;
                            ctx.stroke();
                        }

                        // Boresight line (0°) — slightly more visible
                        if (aDeg === 0) {
                            ctx.beginPath();
                            ctx.moveTo(sensorX, sensorY);
                            ctx.lineTo(sensorX, plotTop);
                            ctx.strokeStyle = "rgba(31, 208, 170, 0.12)";
                            ctx.lineWidth   = 0.8;
                            ctx.stroke();
                        }

                        // Tick
                        ctx.beginPath();
                        ctx.moveTo(ax, plotBottom);
                        ctx.lineTo(ax, plotBottom + 3);
                        ctx.strokeStyle = "#263d60";
                        ctx.lineWidth   = 1;
                        ctx.stroke();

                        ctx.fillStyle = "#91a4c2";
                        ctx.fillText(aDeg + "°", ax, plotBottom + 4);
                    }

                    // ── Sensor icon ───────────────────────────────────────────
                    // Outer ring
                    ctx.beginPath();
                    ctx.arc(sensorX, sensorY, 7, 0, Math.PI * 2);
                    ctx.fillStyle   = "#172640";
                    ctx.fill();
                    ctx.strokeStyle = root.clrAccent;
                    ctx.lineWidth   = 1.5;
                    ctx.stroke();
                    // Inner dot
                    ctx.beginPath();
                    ctx.arc(sensorX, sensorY, 2.8, 0, Math.PI * 2);
                    ctx.fillStyle = root.clrAccent;
                    ctx.fill();
                    // Label
                    ctx.font         = "8px monospace";
                    ctx.textAlign    = "center";
                    ctx.textBaseline = "top";
                    ctx.fillStyle    = "rgba(49,208,170,0.5)";
                    ctx.fillText("IWR6843", sensorX, sensorY + 10);

                    // ── Point cloud ───────────────────────────────────────────
                    if (dashboard.presenceDetected) {
                        // Standard radar coords: Y = forward, X = lateral
                        const azimuthDeg = Math.atan2(dashboard.centroidX, dashboard.centroidY) * 180 / Math.PI;
                        const clampedAz  = Math.max(-40, Math.min(40, azimuthDeg));
                        const tRad       = (clampedAz * Math.PI) / 180;
                        const centerX    = sensorX + Math.tan(tRad) * plotH;
                        const centerY    = rangeToY(Math.max(0.5, Math.min(maxRange, dashboard.distance)));
                        const spreadPx   = Math.min(Math.max(dashboard.spatialSpread * 20, 7), 26);

                        // Radial halo glow
                        const grad = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, spreadPx * 2.2);
                        grad.addColorStop(0.0, "rgba(255, 71, 87, 0.22)");
                        grad.addColorStop(0.6, "rgba(255, 71, 87, 0.06)");
                        grad.addColorStop(1.0, "rgba(255, 71, 87, 0.0)");
                        ctx.beginPath();
                        ctx.arc(centerX, centerY, spreadPx * 2.2, 0, Math.PI * 2);
                        ctx.fillStyle = grad;
                        ctx.fill();

                        // Point cloud dots — FOV-clipped
                        const numPoints = Math.min(Math.floor(dashboard.pointDensity / 12), 22);
                        for (let i = 0; i < numPoints; ++i) {
                            let px, py, valid = false;
                            for (let attempt = 0; attempt < 12; ++attempt) {
                                const angle = Math.random() * Math.PI * 2;
                                const dist  = Math.random() * spreadPx;
                                px = centerX + Math.cos(angle) * dist;
                                py = centerY + Math.sin(angle) * dist;
                                const halfW = Math.tan(fovHalfRad) * (sensorY - py);
                                if (px >= sensorX - halfW && px <= sensorX + halfW
                                        && py >= plotTop && py <= plotBottom) {
                                    valid = true; break;
                                }
                            }
                            if (!valid) continue;
                            const alpha = 0.45 + Math.random() * 0.5;
                            const size  = 1.2 + Math.random() * 2.0;
                            ctx.beginPath();
                            ctx.arc(px, py, size, 0, Math.PI * 2);
                            ctx.fillStyle = "rgba(255, 71, 87, " + alpha.toFixed(2) + ")";
                            ctx.fill();
                        }

                        // Annotation labels
                        ctx.font         = "9px monospace";
                        ctx.textAlign    = "left";
                        ctx.textBaseline = "bottom";
                        ctx.fillStyle    = "rgba(255, 71, 87, 0.9)";
                        ctx.fillText("OCCLUDED", centerX + 4, centerY - spreadPx - 3);

                        ctx.textBaseline = "top";
                        ctx.fillStyle    = "rgba(145, 164, 194, 0.85)";
                        ctx.fillText(
                            dashboard.distance.toFixed(1) + "m  ·  " +
                            dashboard.dopplerVelocity.toFixed(3) + " m/s",
                            centerX + 4, centerY + spreadPx + 2
                        );
                    }
                }

                Connections {
                    target: dashboard
                    function onDetectionsChanged() { radar.requestPaint(); }
                }
                Component.onCompleted: requestPaint()
            }
        }

        // ── UNIFIED METRICS STRIP ─────────────────────────────────────────────
        // All four metrics in one combined rectangle — no separate frames.
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 52
            color: root.clrSurface
            radius: 8
            border.color: root.clrBorder
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 0
                anchors.rightMargin: 0
                spacing: 0

                // ── Metric cell component ──────────────────────────────────────
                component MetricCell: Item {
                    required property string label
                    required property string value
                    required property string unit
                    required property color  valueColor

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 2

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: label
                            color: root.clrMuted
                            font.pixelSize: 9
                            font.weight: Font.DemiBold
                            font.letterSpacing: 0.9
                        }
                        RowLayout {
                            Layout.alignment: Qt.AlignHCenter
                            spacing: 3
                            Text {
                                text: value
                                color: valueColor
                                font.pixelSize: 17
                                font.weight: Font.DemiBold
                            }
                            Text {
                                text: unit
                                color: root.clrMuted
                                font.pixelSize: 9
                                Layout.alignment: Qt.AlignBottom | Qt.AlignLeft
                                bottomPadding: 2
                            }
                        }
                    }
                }

                // ── Four metric cells with dividers ────────────────────────────
                MetricCell {
                    label:      "RANGE"
                    value:      dashboard.distance.toFixed(2)
                    unit:       "m"
                    valueColor: root.clrAccent
                }
                Rectangle { width: 1; Layout.fillHeight: true; color: root.clrBorder; Layout.topMargin: 10; Layout.bottomMargin: 10 }
                MetricCell {
                    label:      "DOPPLER"
                    value:      dashboard.dopplerVelocity.toFixed(3)
                    unit:       "m/s"
                    valueColor: root.clrBlue
                }
                Rectangle { width: 1; Layout.fillHeight: true; color: root.clrBorder; Layout.topMargin: 10; Layout.bottomMargin: 10 }
                MetricCell {
                    label:      "DENSITY"
                    value:      dashboard.pointDensity.toFixed(0)
                    unit:       "pts"
                    valueColor: root.clrPurple
                }
                Rectangle { width: 1; Layout.fillHeight: true; color: root.clrBorder; Layout.topMargin: 10; Layout.bottomMargin: 10 }
                MetricCell {
                    label:      "SNR"
                    value:      dashboard.snr.toFixed(1)
                    unit:       "dB"
                    valueColor: root.clrRed
                }
            }
        }
    }
}
