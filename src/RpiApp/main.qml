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
    readonly property color clrSurface:  Qt.rgba(0x11 / 255, 0x1d / 255, 0x31 / 255, 1)
    readonly property color clrRaised:   Qt.rgba(0x17 / 255, 0x26 / 255, 0x40 / 255, 1)
    readonly property color clrBorder:   Qt.rgba(0x1e / 255, 0x30 / 255, 0x52 / 255, 1)
    readonly property color clrPrimary:  Qt.rgba(0xf4 / 255, 0xf7 / 255, 0xfb / 255, 1)
    readonly property color clrMuted:    Qt.rgba(0x91 / 255, 0xa4 / 255, 0xc2 / 255, 1)
    readonly property color clrAccent:   Qt.rgba(0x31 / 255, 0xd0 / 255, 0xaa / 255, 1)
    readonly property color clrDanger:   Qt.rgba(0xff / 255, 0x47 / 255, 0x57 / 255, 1)
    readonly property color clrBlue:     Qt.rgba(0x73 / 255, 0xa9 / 255, 0xff / 255, 1)
    readonly property color clrPurple:   Qt.rgba(0xb9 / 255, 0xa5 / 255, 0xff / 255, 1)
    readonly property color clrRed:      Qt.rgba(0xff / 255, 0x6b / 255, 0x81 / 255, 1)

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
                    implicitWidth: 22; implicitHeight: 22; radius: 6
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
                    implicitWidth: 1; implicitHeight: 18
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
                    implicitWidth: 1; implicitHeight: 18
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

                        // Radial guide-line (except boresight — drawn separately)
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
                    const pts = dashboard.pointCloud;
                    if (pts.length > 0) {
                        const pxPerM = plotH / maxRange;   // same scale as the range rings

                        for (const p of pts) {
                            // Standard radar coords: x = lateral, y = forward
                            // (range), z = height (unused for now).
                            const px = sensorX + p.x * pxPerM;
                            const py = rangeToY(p.y);

                            // Clip to the FOV sector and the plot area.
                            const halfW = Math.tan(fovHalfRad) * (sensorY - py);
                            if (px < sensorX - halfW || px > sensorX + halfW) continue;
                            if (py < plotTop || py > plotBottom) continue;

                            // Doppler colour: approaching = red, receding = blue,
                            // stationary = white.
                            let fill;
                            if (p.v >  0.05)      fill = "rgba(255, 71, 87, 0.9)";
                            else if (p.v < -0.05) fill = "rgba(115, 169, 255, 0.9)";
                            else                  fill = "rgba(244, 247, 251, 0.85)";

                            ctx.beginPath();
                            ctx.arc(px, py, 2.2, 0, Math.PI * 2);
                            ctx.fillStyle = fill;
                            ctx.fill();
                        }
                    }
                }

                Connections {
                    target: dashboard
                    function onDetectionsChanged() { radar.requestPaint(); }
                    function onPointCloudChanged() { radar.requestPaint(); }
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
                    id: cell
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
                            text: cell.label
                            color: "#91a4c2"
                            font.pixelSize: 9
                            font.weight: Font.DemiBold
                            font.letterSpacing: 0.9
                        }
                        RowLayout {
                            Layout.alignment: Qt.AlignHCenter
                            spacing: 3
                            Text {
                                text: cell.value
                                color: cell.valueColor
                                font.pixelSize: 17
                                font.weight: Font.DemiBold
                            }
                            Text {
                                text: cell.unit
                                color: "#91a4c2"
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
                Rectangle { implicitWidth: 1; Layout.fillHeight: true; color: root.clrBorder; Layout.topMargin: 10; Layout.bottomMargin: 10 }
                MetricCell {
                    label:      "DOPPLER"
                    value:      dashboard.dopplerVelocity.toFixed(3)
                    unit:       "m/s"
                    valueColor: root.clrBlue
                }
                Rectangle { implicitWidth: 1; Layout.fillHeight: true; color: root.clrBorder; Layout.topMargin: 10; Layout.bottomMargin: 10 }
                MetricCell {
                    label:      "DENSITY"
                    value:      dashboard.pointDensity.toFixed(0)
                    unit:       "pts"
                    valueColor: root.clrPurple
                }
                Rectangle { implicitWidth: 1; Layout.fillHeight: true; color: root.clrBorder; Layout.topMargin: 10; Layout.bottomMargin: 10 }
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
