#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "DashboardState.h"

int main(int argc, char *argv[]) {
    const QGuiApplication app(argc, argv);
    DashboardState dashboard;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("dashboard", &dashboard);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/DesktopApp/main.qml")));
    return app.exec();
}
