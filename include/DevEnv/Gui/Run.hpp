#pragma once

#include <filesystem>

class QQmlEngine;
class QQuickWindow;

namespace Sarwave::DevEnv::Gui {

/// Replays the last recorded session into the MaximalGui dashboard, looping, while printing every
/// frame with the DebugLogger. What TestRpiApp's window runs.
struct RunRecorded {
    /// Where the live app records to and the recorded app replays from. A fixed place in the
    /// user's local application data (Environment.SpecialFolder.LocalApplicationData:
    /// ~/.local/share or $XDG_DATA_HOME on Linux, %LOCALAPPDATA% on Windows), in a "Sarwave"
    /// folder, so both apps reach the same file no matter which folder each one is started
    /// from. The folder is created on first use.
    static const std::filesystem::path& sessionFile();

    /// Call once the window is shown. Throws (FileNotFoundException, InvalidDataException)
    /// when the session file can't be loaded.
    static void onOpened(QQuickWindow& window, QQmlEngine& engine);

    static void onClosed();
};

/// The live run: MinimalGui dashboard, port dialogue, cfg dialogue (sent without sensorStart),
/// then captures frames -- recording them to RunRecorded::sessionFile() -- and starts the sensor.
struct RunStuff {
    /// Call once the window is shown. Fails the process (like Environment.FailFast) on error.
    static void onOpened(QQuickWindow& window, QQmlEngine& engine);

    static void onClosed();
};

}  // namespace Sarwave::DevEnv::Gui
