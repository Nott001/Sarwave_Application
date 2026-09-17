#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "DashboardState.h"
#include "MMWave/Porter.hpp"
#include "MMWave/Prompting/Terminal.hpp"
#include "QtCommon/MMWavePrompt/PortDialog.hpp"
#include "QtCommon/MMWavePrompt/CfgDialog.hpp"

int main(int argc, char *argv[]) {
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
    } else {
        MMWave::Prompting::Gui::cfgGUIAndSend(mmwave);
    }

    DashboardState dashboard;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("dashboard", &dashboard);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },Qt::QueuedConnection);

    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/RpiApp/main.qml")));
    return app.exec();
}
