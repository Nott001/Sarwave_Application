#include <QStandardPaths>
#include <QString>
#include <memory>

#include "DevEnv/Gui/Run.hpp"
#include "Gui/Models/RadarData.hpp"
#include "Gui/Views/GuiHost.hpp"
#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Configured/PeopleTracking/DebugLogger.hpp"
#include "MMWave/Recording/FramePlayback.hpp"
#include "MMWave/Recording/FrameRecording.hpp"

namespace Sarwave::DevEnv::Gui {
namespace {

using namespace Sarwave::Core;
namespace CoreGui = Sarwave::Core::Gui;
using MMWave::Configured::PeopleTracking::DebugLogger;
using MMWave::Recording::FramePlayback;
using MMWave::Recording::FrameRecording;

/// How long point clouds stay on the radar grid.
const MMWave::TimeSpan PointRetention = MMWave::TimeSpans::fromSeconds(1);

// Process-lifetime state, as the C# static fields. Declared so that destruction at exit (reverse
// order) stops the playback before anything it uses goes away.
std::unique_ptr<CoreGui::Models::RadarData> g_data;
std::unique_ptr<FrameRecording> g_recording;
DebugLogger g_debugLogger(DebugLogger::Config{.hideBytes = true});
std::unique_ptr<FramePlayback> g_playback;

}  // namespace

const std::filesystem::path& RunRecorded::sessionFile() {
    static const std::filesystem::path file = [] {
        const QString localData =
            QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        const std::filesystem::path folder =
            std::filesystem::path(localData.toStdU16String()) / "Sarwave";
        std::filesystem::create_directories(folder);
        return folder / "default_session.swrec";
    }();
    return file;
}

void RunRecorded::onOpened(QQuickWindow& window, QQmlEngine& engine) {
    // The dashboard gets its own view model up front.
    g_data = std::make_unique<CoreGui::Models::RadarData>(PointRetention);
    CoreGui::Views::showMaximalGui(window, engine, *g_data);

    g_recording = std::make_unique<FrameRecording>(FrameRecording::load(sessionFile()));
    g_playback = std::make_unique<FramePlayback>(*g_recording);
    g_playback->loop = true;
    g_playback->begin(
        [data = g_data.get()](const MMWave::Streaming::Frame& frame, MMWave::TimeSpan timestamp) {
            g_debugLogger.printFrameDataVerbose(frame, timestamp);
            data->update(frame, timestamp);
        });
}

void RunRecorded::onClosed() {
    if (g_playback) g_playback->dispose();    // stops playback and waits for it
    if (g_recording) g_recording->dispose();  // then frees the frames
}

}  // namespace Sarwave::DevEnv::Gui
