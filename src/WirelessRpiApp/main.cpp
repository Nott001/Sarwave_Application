// Sarwave.Source.WirelessRpiApp: shows the radar dashboard of a session the live app streams over
// the LAN (RunLan), e.g. on a laptop beside the handheld device; no sensor needed.
// Deliberately bare, as the C# app is: one empty window, which RunLan fills once it is
// shown and cleans up after when it closes.

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
    using namespace Sarwave::DevEnv::Gui;

    Sarwave::Core::Gui::Views::initialize();
    Sarwave::DevEnv::initialize();

    QQmlEngine engine;
    QQuickWindow window;
    window.setTitle(QStringLiteral("Sarwave Wireless"));
    // The C# window sets no size; the platform picks one. Three quarters of the screen.
    if (QScreen* screen = QGuiApplication::primaryScreen())
        window.resize(screen->availableSize() * 3 / 4);
    window.show();

    // Window.Opened: runs once the window is on screen.
    auto opened = std::make_shared<QMetaObject::Connection>();
    *opened = QObject::connect(&window, &QQuickWindow::frameSwapped, &window, [&, opened] {
        QObject::disconnect(*opened);
        QMetaObject::invokeMethod(
            &window,
            [&] {
                try {
                    RunLan::onOpened(window, engine);
                } catch (...) {
                    // An exception escaping OnOpened is unhandled in the C# app too.
                    Sarwave::Core::failUnhandled(std::current_exception());
                }
            },
            Qt::QueuedConnection);
    });

    const int exitCode = QGuiApplication::exec();
    RunLan::onClosed();  // Window.Closed
    return exitCode;
}
