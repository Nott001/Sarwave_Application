// Renders the dashboards to PNG files without a display (QT_QPA_PLATFORM=offscreen works):
//   SarwaveGuiPreview <out dir> [recording.swrec]
// Writes maximal-design.png and minimal-design.png from the previewer's sample data and, given a
// recording, maximal-live.png / minimal-live.png after feeding it its first 60 frames.

#include <QDir>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QImage>
#include <QQmlEngine>
#include <QQuickWindow>
#include <iostream>
#include <memory>

#include "Gui/Models/RadarData.hpp"
#include "Gui/Views/GuiHost.hpp"
#include "MMWave/Recording/FrameRecording.hpp"

namespace {

void settle(int ms) {
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < ms) QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}

void save(QQuickWindow& window, const QString& path) {
    settle(300);
    window.grabWindow().save(path);
    std::cout << "wrote " << path.toStdString() << "\n";
}

}  // namespace

int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);
    if (argc < 2) {
        std::cerr << "usage: SarwaveGuiPreview <out dir> [recording.swrec]\n";
        return 2;
    }
    const QDir out(QString::fromLocal8Bit(argv[1]));
    namespace Views = Sarwave::Core::Gui::Views;
    Views::initialize();
    QQmlEngine engine;

    QQuickWindow window;
    window.resize(1100, 600);
    window.show();
    Views::showMaximalGuiDesign(window, engine);
    save(window, out.filePath("maximal-design.png"));

    window.resize(800, 450);
    Views::showMinimalGuiDesign(window, engine);
    save(window, out.filePath("minimal-design.png"));

    if (argc > 2) {
        using namespace Sarwave::Core;
        auto recording = MMWave::Recording::FrameRecording::load(argv[2]);
        Gui::Models::RadarData data(MMWave::TimeSpans::fromSeconds(1));

        window.resize(1100, 600);
        Views::showMaximalGui(window, engine, data);
        for (int i = 0; i < std::min(60, recording.count()); ++i) {
            data.update(recording.frames()[static_cast<std::size_t>(i)],
                        recording.timestamps()[static_cast<std::size_t>(i)]);
        }
        save(window, out.filePath("maximal-live.png"));

        window.resize(800, 450);
        Views::showMinimalGui(window, engine, data);
        for (int i = 60; i < std::min(120, recording.count()); ++i) {
            data.update(recording.frames()[static_cast<std::size_t>(i)],
                        recording.timestamps()[static_cast<std::size_t>(i)]);
        }
        save(window, out.filePath("minimal-live.png"));
    }
    return 0;
}
