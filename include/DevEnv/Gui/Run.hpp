#pragma once

class QQmlEngine;
class QQuickWindow;

namespace Sarwave::DevEnv::Gui {

/// Replays the last recorded session into the MaximalGui dashboard, looping, while printing every
/// frame with the DebugLogger. What TestRpiApp's window runs.
struct RunRecorded {
    /// The file RunStuff records to and RunRecorded plays back, in the working directory.
    static constexpr const char* SessionFile = "default_session.swrec";

    /// Call once the window is shown. Throws (FileNotFoundException, InvalidDataException)
    /// when the session file can't be loaded.
    static void onOpened(QQuickWindow& window, QQmlEngine& engine);

    static void onClosed();
};

/// The live run: MinimalGui dashboard, port dialogue, cfg dialogue (sent without sensorStart),
/// then captures frames -- recording them to RunRecorded::SessionFile -- and starts the sensor.
struct RunStuff {
    /// Call once the window is shown. Fails the process (like Environment.FailFast) on error.
    static void onOpened(QQuickWindow& window, QQmlEngine& engine);

    static void onClosed();
};

}  // namespace Sarwave::DevEnv::Gui
