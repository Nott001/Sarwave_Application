#pragma once

#include <atomic>
#include <condition_variable>
#include <exception>
#include <functional>
#include <mutex>
#include <thread>

#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"
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

    /// Called in place of the frame callback's normal return when a frame is malformed, as with
    /// ConstantCapture: either the frame callback threw a FrameMalformedException for the frame
    /// it was given, or the recording holds a note that the reader skipped one.
    /// timestamp: on the same clock as the frame callback's timestamps.
    using MalformedCallback = std::function<void(
        const Exceptions::FrameMalformedException& exception, TimeSpan timestamp)>;

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
    ///
    /// onMalformed: without one, a FrameMalformedException from onFrame ends playback like any
    /// other exception, and the recording's notes are not replayed.
    void begin(FrameCallback onFrame, MalformedCallback onMalformed = {});

    /// The timestamp in the recording that a delivered `timestamp` stands for: the same on the
    /// first pass, with the looping offset taken off on later ones. Use it to look up anything
    /// else recorded on the same clock, such as the session's IMU samples.
    [[nodiscard]] TimeSpan recordedTimestamp(TimeSpan timestamp) const;

    /// Signals playback to stop and blocks until it has (only signals when called from inside
    /// the frame callback).
    void stop();

    /// Same as stop().
    void dispose() {
        stop();
    }

   private:
    void playbackLoop(FrameCallback onFrame, MalformedCallback onMalformed, double speed,
                      bool loop);
    void waitUntil(std::chrono::steady_clock::time_point start, TimeSpan due);
    void notify(const std::function<void()>& callback);

    /// One pass lasts the recording's duration plus one typical frame gap, so a loop's first
    /// frame doesn't land on top of the previous pass's last one.
    [[nodiscard]] TimeSpan passLength() const;
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
