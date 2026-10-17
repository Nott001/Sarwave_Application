#include <QDir>
#include <QStandardPaths>
#include <memory>
#include <optional>

#include "DevEnv/Gui/ImuFrames.hpp"
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
std::optional<RecordedGate> g_gate;
DebugLogger g_debugLogger(DebugLogger::Config{.hideBytes = true});
std::unique_ptr<FramePlayback> g_playback;

}  // namespace

const std::filesystem::path& RunRecorded::sessionFile() {
    static const std::filesystem::path path = [] {
        // GenericDataLocation is what .NET's LocalApplicationData resolves to: ~/.local/share
        // (or $XDG_DATA_HOME) on Linux, %LOCALAPPDATA% on Windows.
        const QString root = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        const QString folder = QDir(root).filePath(QStringLiteral("Sarwave"));
        QDir().mkpath(folder);
        return std::filesystem::path(
            QDir(folder).absoluteFilePath("default_session.swrec").toStdString());
    }();
    return path;
}

void RunRecorded::onOpened(QQuickWindow& window, QQmlEngine& engine) {
    // The dashboard gets its own view model up front.
    g_data = std::make_unique<CoreGui::Models::RadarData>(PointRetention);
    CoreGui::Views::showMaximalGui(window, engine, *g_data);

    g_recording = std::make_unique<FrameRecording>(FrameRecording::load(sessionFile()));
    g_gate = ImuFrames::loadRecordedGate(sessionFile());
    g_playback = std::make_unique<FramePlayback>(*g_recording);
    g_playback->loop = true;

    Imu::Gating::MotionGate* gate = g_gate ? g_gate->gate.get() : nullptr;
    FramePlayback* playback = g_playback.get();
    g_playback->begin(
        [data = g_data.get(), gate, playback](const MMWave::Streaming::Frame& frame,
                                              MMWave::TimeSpan timestamp) {
            g_debugLogger.printFrameDataVerbose(frame, timestamp);

            // The IMU recording is on the recording's clock, which a looping playback has moved
            // on from.
            std::optional<Imu::Gating::ImuFrameState> imu;
            if (gate) imu = gate->evaluate(playback->recordedTimestamp(timestamp));
            ImuFrames::apply(*data, frame, timestamp, imu);
        },
        [data = g_data.get()](const MMWave::Exceptions::FrameMalformedException&,
                              MMWave::TimeSpan timestamp) { data->skipFrame(timestamp); });
}

void RunRecorded::onClosed() {
    if (g_playback) g_playback->dispose();    // stops playback and waits for it
    if (g_recording) g_recording->dispose();  // then frees the frames
}

}  // namespace Sarwave::DevEnv::Gui
