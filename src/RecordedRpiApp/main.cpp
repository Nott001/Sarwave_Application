// Sarwave.Source.RecordedRpiApp: replays a recorded session on the radar dashboard; no sensor
// needed. Deliberately bare, as the C# project: no QML of its own, no view models. The app is one
// empty window, and RunRecorded fills it when it opens and cleans up when it closes.

#include <QGuiApplication>
#include <QMetaObject>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QScreen>
#include <exception>
#include <memory>

#include "DevEnv/DevEnv.hpp"
#include "DevEnv/Gui/Run.hpp"
#include "Gui/Views/GuiHost.hpp"
#include "MMWave/Common/Exceptions.hpp"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    Sarwave::Core::Gui::Views::initialize();
    Sarwave::DevEnv::initialize();

    QQmlEngine engine;
    QQuickWindow window;
    window.setTitle(QStringLiteral("Sarwave Recorded"));
    // The C# window sets no size and the platform picks one. Three quarters of the screen, as
    // TestRpiApp's MainWindow.
    window.resize(window.screen()->availableSize() * 0.75);
    window.show();

    using Sarwave::DevEnv::Gui::RunRecorded;

    // window.Opened: runs once the window is on screen.
    auto opened = std::make_shared<QMetaObject::Connection>();
    *opened = QObject::connect(&window, &QQuickWindow::frameSwapped, &window, [&, opened] {
        QObject::disconnect(*opened);
        QMetaObject::invokeMethod(
            &window,
            [&] {
                try {
                    RunRecorded::onOpened(window, engine);
                } catch (...) {
                    // An exception escaping the Opened handler is unhandled in the C# app too.
                    Sarwave::Core::failUnhandled(std::current_exception());
                }
            },
            Qt::QueuedConnection);
    });

    const int exitCode = QGuiApplication::exec();

    // window.Closed
    RunRecorded::onClosed();

    return exitCode;
}
