#pragma once

#include <atomic>
#include <condition_variable>
#include <exception>
#include <functional>
#include <mutex>
#include <thread>

#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Recording/FrameRecording.hpp"
#include "MMWave/Streaming/Frame.hpp"

namespace Sarwave::Core::MMWave::Recording {

/// Replays a FrameRecording at the pace it was recorded: each frame is handed to the callback
/// when its recorded timestamp comes due, on a background thread -- the same shape as
/// ConstantCapture, so it can stand in for a live sensor.
///
/// It does not own the recording; keep that alive (undisposed) while playback is running.
class FramePlayback {
   public:
    /// frame: owned by the recording; don't keep it beyond the call.
    /// timestamp: the frame's recorded timestamp. When looping, later passes continue
    /// counting up from the previous pass instead of jumping back.
    using FrameCallback = std::function<void(const Streaming::Frame& frame, TimeSpan timestamp)>;

    explicit FramePlayback(const FrameRecording& recording) : m_recording(recording) {
    }

    ~FramePlayback();
    FramePlayback(const FramePlayback&) = delete;
    FramePlayback& operator=(const FramePlayback&) = delete;

    /// 1 = as recorded, 2 = twice as fast, 0.5 = half speed. Read when playback begins.
    double speed = 1;

    /// Start over after the last frame instead of finishing. Read when playback begins.
    bool loop = false;

    /// The exception that ended the last playback, if it ended by failing.
    [[nodiscard]] std::exception_ptr error() const;

    [[nodiscard]] bool isPlaying() const;

    /// Blocks until playback ends: last frame delivered, stop() called, or the callback threw
    /// (see error()). Returns immediately when nothing is playing.
    void waitForCompletion();

    /// Throws InvalidOperationException if already playing or `speed` is not a positive,
    /// finite number.
    void begin(FrameCallback onFrame);

    /// Signals playback to stop and blocks until it has (only signals when called from inside
    /// the frame callback).
    void stop();

    /// Same as stop().
    void dispose() {
        stop();
    }

   private:
    void playbackLoop(FrameCallback onFrame, double speed, bool loop);
    bool waitUntil(std::chrono::steady_clock::time_point due);
    void joinFinishedThread();

    const FrameRecording& m_recording;

    mutable std::mutex m_gate;
    std::condition_variable m_wake;      // wakes the playback thread early on stop()
    std::condition_variable m_finished;  // signals waiters when playback ends
    std::thread m_thread;
    bool m_playing = false;          // guarded by m_gate
    bool m_cancelRequested = false;  // guarded by m_gate
    std::exception_ptr m_error;      // guarded by m_gate
    std::atomic<std::thread::id> m_callbackThreadId{};
};

}  // namespace Sarwave::Core::MMWave::Recording
