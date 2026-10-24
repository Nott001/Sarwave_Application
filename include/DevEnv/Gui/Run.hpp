#pragma once

#include <filesystem>

class QQmlEngine;
class QQuickWindow;

namespace Sarwave::DevEnv::Gui {

/// Replays the last recorded session into the MaximalGui dashboard, looping, while printing every
/// frame with the DebugLogger. Gated and levelled with the session's IMU recording, if it has
/// one, exactly as it was live. What TestRpiApp's and RecordedRpiApp's windows run.
struct RunRecorded {
    /// Where the live app records to and the recorded app replays from. A fixed place in the
    /// user's local application data (~/.local/share/Sarwave on Linux, %LOCALAPPDATA%\Sarwave on
    /// Windows -- the same file the C# apps use), so both apps reach the same file no matter
    /// which folder each one is started from. The folder is created on first use.
    static const std::filesystem::path& sessionFile();

    /// Call once the window is shown. Throws (FileNotFoundException, InvalidDataException)
    /// when the session file can't be loaded.
    static void onOpened(QQuickWindow& window, QQmlEngine& engine);

    static void onClosed();
};

/// The live run: MinimalGui dashboard, port dialogue, cfg dialogue (sent without sensorStart),
/// LAN dialogue (stream the session or not), then captures frames -- recording them, and the IMU
/// beside them, to RunRecorded::sessionFile(), and streaming both if asked -- and starts the
/// sensor. Each frame is gated and levelled by the IMU (see ImuFrames), and what is held from
/// earlier frames is moved with the device.
struct RunStuff {
    /// Call once the window is shown. Fails the process (like Environment.FailFast) on error.
    ///
    /// printFrames: also print every frame in full to the terminal (the C# parameter is named
    /// `save`). Off by default: on a Raspberry Pi the printing is slow enough to matter.
    static void onOpened(QQuickWindow& window, QQmlEngine& engine, bool printFrames = false);

    static void onClosed();

    /// Every 10 seconds, print how long the worker took per frame, how long frames waited for it,
    /// and how long drawing the radar view took (LiveRpiApp --timing). Set before onOpened. The
    /// measuring costs well under a microsecond per frame and never touches the reader thread.
    static bool timingReport;
};

/// The wireless app: shows the MaximalGui dashboard of a session the live app streams over the
/// LAN (see RunStuff). Asks for the sender's address, then connects, and connects again whenever
/// the connection is lost; the window title says how it stands.
///
/// What arrives is processed the way a replay is: the raw IMU samples go through this app's own
/// ImuFrames::options() (so the mounting set there must match the sender's device), and each
/// frame is gated, levelled and moved with the device exactly as it is live.
struct RunLan {
    /// Call once the window is shown. Fails the process (like Environment.FailFast) on error.
    static void onOpened(QQuickWindow& window, QQmlEngine& engine);

    static void onClosed();
};

}  // namespace Sarwave::DevEnv::Gui
