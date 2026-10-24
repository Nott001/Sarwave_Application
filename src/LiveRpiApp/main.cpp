// Sarwave.Source.LiveRpiApp: shows the radar dashboard from a sensor on the serial ports, and
// records the session (frames, malformed-frame notes and the IMU beside them) to
// RunRecorded::sessionFile(). Deliberately bare, as the C# app is: one empty window, which
// RunStuff fills once it is shown and cleans up after when it closes.
//
//   LiveRpiApp [--imu=auto|off|required] [--print-frames] [--timing]
//
// The first two are what the C# app sets in code: ImuFrames.Mode (App.cs) and RunStuff's `save`
// argument (print every frame to the terminal; off, as the C# app passes false). --timing prints
// how long processing and drawing take every 10 seconds (RunStuff::timingReport).

#include <QGuiApplication>
#include <QMetaObject>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QScreen>
#include <QStringList>
#include <exception>
#include <iostream>
#include <memory>

#include "DevEnv/DevEnv.hpp"
#include "DevEnv/Gui/Run.hpp"
#include "Gui/Views/GuiHost.hpp"
#include "Integration/PeopleTracking/ImuFrames.hpp"
#include "MMWave/Common/Exceptions.hpp"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    using namespace Sarwave::DevEnv::Gui;
    using namespace Sarwave::Integration::PeopleTracking;

    bool printFrames = false;
    for (const QString& arg : QGuiApplication::arguments().mid(1)) {
        if (arg == QStringLiteral("--print-frames")) {
            printFrames = true;
        } else if (arg == QStringLiteral("--timing")) {
            RunStuff::timingReport = true;
        } else if (arg.startsWith(QStringLiteral("--imu="))) {
            const auto mode = parseImuMode(arg.mid(6).toStdString());
            if (!mode) {
                std::cerr << "--imu takes auto, off or required." << std::endl;
                return 2;
            }
            ImuFrames::mode = *mode;
        } else {
            std::cerr << "Usage: LiveRpiApp [--imu=auto|off|required] [--print-frames] [--timing]"
                      << std::endl;
            return 2;
        }
    }

    Sarwave::Core::Gui::Views::initialize();
    Sarwave::DevEnv::initialize();

    QQmlEngine engine;
    QQuickWindow window;
    window.setTitle(QStringLiteral("Sarwave Live"));
    // The C# window sets no size; the platform picks one. Three quarters of the screen.
    if (QScreen* screen = QGuiApplication::primaryScreen())
        window.resize(screen->availableSize() * 3 / 4);
    window.show();

    // Window.Opened: runs once the window is on screen.
    auto opened = std::make_shared<QMetaObject::Connection>();
    *opened =
        QObject::connect(&window, &QQuickWindow::frameSwapped, &window, [&, opened, printFrames] {
            QObject::disconnect(*opened);
            QMetaObject::invokeMethod(
                &window, [&, printFrames] { RunStuff::onOpened(window, engine, printFrames); },
                Qt::QueuedConnection);
        });

    const int exitCode = QGuiApplication::exec();
    RunStuff::onClosed();  // Window.Closed
    return exitCode;
}
