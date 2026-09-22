#include <cmath>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QVariantList>
#include <QVariantMap>

#include "DashboardState.h"
#include "MMWave/Porter.hpp"
#include "MMWave/Prompting/Terminal.hpp"
#include "MMWave/ThreadedStreaming.hpp"
#include "MMWave/Tlv.hpp"
#include "QtCommon/MMWavePrompt/CfgDialog.hpp"
#include "QtCommon/MMWavePrompt/PortDialog.hpp"

int main(int argc, char* argv[])
{
    const QGuiApplication app(argc, argv);

    auto mmwave = [&]() -> MMWave::Porter::Context {
        if (argc >= 3) {
            const int cli_port = std::stoi(argv[1]);
            const int data_port = std::stoi(argv[2]);
            return MMWave::Porter::openMMWavePorts(
                MMWave::Prompting::General::createPath(cli_port, data_port));
        }
        return MMWave::Prompting::Gui::portGUI();
    }();

    if (argc >= 4) {
        MMWave::Prompting::Gui::cfgGUIAndSend(mmwave, argv[3]);
    }
    else {
        MMWave::Prompting::Gui::cfgGUIAndSend(mmwave);
    }

    DashboardState dashboard;

    // Captures frames on a background thread and feeds each frame's point
    // cloud into the dashboard. The dashboard writes happen on the main thread
    // (where the dispatcher QObject below lives), so the capture callback only
    // marshals a copy of the frame across.
    QObject frameDispatcher;
    MMWave::Streaming::ConstantCapture capture(mmwave);
    capture.beginConstantCapture(
        [&frameDispatcher, &dashboard](const MMWave::Streaming::Frame& frame) {
            QMetaObject::invokeMethod(
                &frameDispatcher,
                [&dashboard, frame = frame]() {
                    QVariantList points;

                    // Point Cloud TLV (1020) is compressed: one PointUnit of
                    // scale factors followed by 8-byte CompressedPoint entries.
                    // Convert back to real units, then to the dashboard's
                    // radar coords (x = lateral, y = forward range, z = height).
                    for (const auto& tlv : MMWave::Tlv::TlvRange(frame)) {
                        if (tlv.type != MMWave::Tlv::TLV_POINT_CLOUD) continue;

                        const auto cloud = tlv.compressedPoints();
                        const auto& units = cloud.unit();
                        for (const auto& point : cloud) {
                            const double rangeM = point.range * units.rangeUnit;
                            const double azimuth = point.azimuth * units.azimuthUnit;
                            const double elevation = point.elevation * units.elevationUnit;

                            QVariantMap entry;
                            entry.insert("x", rangeM * std::sin(azimuth));
                            entry.insert("y", rangeM * std::cos(azimuth));
                            entry.insert("z", rangeM * std::sin(elevation));
                            entry.insert("v", static_cast<double>(point.doppler) * units.dopplerUnit);
                            points.append(entry);
                        }
                    }

                    dashboard.updatePointCloud(points);
                },
                Qt::QueuedConnection);
        });

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("dashboard", &dashboard);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/RpiApp/main.qml")));
    return app.exec();
}
