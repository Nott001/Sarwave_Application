#include "MMWave/Recording/FramePlayback.hpp"

#include <cmath>

#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::Core::MMWave::Recording {
namespace {

struct Cancelled {};

}  // namespace

FramePlayback::~FramePlayback() {
    stop();
    if (m_thread.joinable()) m_thread.detach();  // only when destroyed from inside its callback
}

std::exception_ptr FramePlayback::error() const {
    std::lock_guard lock(m_gate);
    return m_error;
}

bool FramePlayback::isPlaying() const {
    std::lock_guard lock(m_gate);
    return m_playing;
}

void FramePlayback::waitForCompletion() {
    std::unique_lock lock(m_gate);
    m_finished.wait(lock, [this] { return !m_playing; });
}

void FramePlayback::begin(FrameCallback onFrame) {
    std::lock_guard lock(m_gate);
    if (m_playing) throw InvalidOperationException("FramePlayback is already playing");

    const double playbackSpeed = speed;
    if (!(playbackSpeed > 0) || std::isinf(playbackSpeed)) {
        throw InvalidOperationException("Speed must be a positive, finite number.");
    }

    joinFinishedThread();
    m_error = nullptr;
    m_cancelRequested = false;
    m_playing = true;
    m_thread = std::thread([this, onFrame = std::move(onFrame), playbackSpeed, shouldLoop = loop] {
        playbackLoop(onFrame, playbackSpeed, shouldLoop);
    });
}

void FramePlayback::stop() {
    std::unique_lock lock(m_gate);
    if (!m_playing) {
        joinFinishedThread();
        return;
    }

    m_cancelRequested = true;
    m_wake.notify_all();

    // Stop called from inside the frame callback: can't wait for ourselves.
    if (m_callbackThreadId.load() == std::this_thread::get_id()) return;

    m_finished.wait(lock, [this] { return !m_playing; });
    joinFinishedThread();
}

void FramePlayback::joinFinishedThread() {
    if (m_thread.joinable() && m_thread.get_id() != std::this_thread::get_id()) m_thread.join();
}

bool FramePlayback::waitUntil(std::chrono::steady_clock::time_point due) {
    std::unique_lock lock(m_gate);
    m_wake.wait_until(lock, due, [this] { return m_cancelRequested; });
    return !m_cancelRequested;
}

void FramePlayback::playbackLoop(FrameCallback onFrame, double playbackSpeed, bool shouldLoop) {
    std::exception_ptr failure;
    try {
        const int count = m_recording.count();
        if (count > 0) {
            const auto timestamps = m_recording.timestamps();
            const TimeSpan first = timestamps[0];
            const TimeSpan recordingDuration = m_recording.duration();

            // One pass lasts the recording's duration plus one typical frame gap, so a loop's
            // first frame doesn't land on top of the previous pass's last one.
            const TimeSpan passLength =
                recordingDuration +
                (count > 1 ? TimeSpans::divide(recordingDuration, count - 1) : TimeSpan::zero());

            // Every frame is scheduled against this one clock, so late frames don't push the
            // rest of the recording back.
            const auto start = std::chrono::steady_clock::now();

            for (int pass = 0;; pass++) {
                const TimeSpan passOffset = TimeSpans::multiply(passLength, pass);

                for (int i = 0; i < count; i++) {
                    const TimeSpan timestamp = timestamps[static_cast<std::size_t>(i)] + passOffset;
                    const TimeSpan due = TimeSpans::divide(timestamp - first, playbackSpeed);
                    const auto dueAt =
                        start +
                        std::chrono::duration_cast<std::chrono::steady_clock::duration>(due);
                    if (dueAt > std::chrono::steady_clock::now() && !waitUntil(dueAt))
                        throw Cancelled{};

                    {
                        std::lock_guard lock(m_gate);
                        if (m_cancelRequested) throw Cancelled{};
                    }

                    m_callbackThreadId.store(std::this_thread::get_id());
                    try {
                        onFrame(m_recording.frames()[static_cast<std::size_t>(i)], timestamp);
                    } catch (...) {
                        m_callbackThreadId.store(std::thread::id());
                        throw;
                    }
                    m_callbackThreadId.store(std::thread::id());
                }

                if (!shouldLoop) break;
            }
        }
    } catch (const Cancelled&) {
        // normal shutdown
    } catch (...) {
        failure = std::current_exception();
    }

    std::lock_guard lock(m_gate);
    m_error = failure;
    m_playing = false;
    m_finished.notify_all();
}

}  // namespace Sarwave::Core::MMWave::Recording
