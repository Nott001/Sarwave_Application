// Sarwave.Source.TestRpiApp: opens MainWindow and, once it is shown, hands it to the dev
// environment's run -- RunRecorded (replay default_session.swrec into the MaximalGui dashboard
// while logging every frame), exactly as the C# MainWindow.OnOpened does.
//
//   TestRpiApp          replay the recorded session (the C# app's behaviour)
//   TestRpiApp --live   the live sensor run instead (RunStuff: port + cfg dialogues, MinimalGui,
//                       recording to default_session.swrec). In the C# app this is the
//                       commented-out alternative in MainWindow.axaml.cs.

#include <QGuiApplication>
#include <QIcon>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QStringList>
#include <exception>
#include <memory>

#include "DevEnv/DevEnv.hpp"
#include "DevEnv/Gui/Run.hpp"
#include "Gui/Views/GuiHost.hpp"
#include "MMWave/Common/Exceptions.hpp"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/TestRpiApp/avalonia-logo.ico")));

    const bool live = QGuiApplication::arguments().contains(QStringLiteral("--live"));

    Sarwave::Core::Gui::Views::initialize();
    Sarwave::DevEnv::initialize();

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("TestRpiApp", "MainWindow");
    if (engine.rootObjects().isEmpty()) return -1;

    auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().constFirst());
    if (!window) return -1;

    using namespace Sarwave::DevEnv::Gui;

    // OnOpened: runs once the window is on screen.
    auto opened = std::make_shared<QMetaObject::Connection>();
    *opened = QObject::connect(window, &QQuickWindow::frameSwapped, window, [=, &engine] {
        QObject::disconnect(*opened);
        QMetaObject::invokeMethod(
            window,
            [=, &engine] {
                try {
                    if (live)
                        RunStuff::onOpened(*window, engine);
                    else
                        RunRecorded::onOpened(*window, engine);
                } catch (...) {
                    // An exception escaping OnOpened is unhandled in the C# app too.
                    Sarwave::Core::failUnhandled(std::current_exception());
                }
            },
            Qt::QueuedConnection);
    });

    const int exitCode = QGuiApplication::exec();

    // OnClosed
    if (live)
        RunStuff::onClosed();
    else
        RunRecorded::onClosed();

    return exitCode;
}
