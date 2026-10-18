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
/// then captures frames -- recording them, and the IMU beside them, to RunRecorded::sessionFile()
/// -- and starts the sensor. Each frame is gated and levelled by the IMU (see ImuFrames).
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

}  // namespace Sarwave::DevEnv::Gui
