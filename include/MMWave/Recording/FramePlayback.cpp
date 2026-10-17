#include "MMWave/Recording/FramePlayback.hpp"

#include <algorithm>
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

void FramePlayback::begin(FrameCallback onFrame, MalformedCallback onMalformed) {
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
    m_thread = std::thread(
        [this, onFrame = std::move(onFrame), onMalformed = std::move(onMalformed), playbackSpeed,
         shouldLoop = loop] { playbackLoop(onFrame, onMalformed, playbackSpeed, shouldLoop); });
}

TimeSpan FramePlayback::recordedTimestamp(TimeSpan timestamp) const {
    const TimeSpan pass = passLength();
    if (pass <= TimeSpan::zero()) return timestamp;

    const std::int64_t passes =
        std::max<std::int64_t>(0, (timestamp - m_recording.timestamps()[0]).count() / pass.count());
    return timestamp - pass * passes;
}

TimeSpan FramePlayback::passLength() const {
    const int count = m_recording.count();
    if (count == 0) return TimeSpan::zero();
    if (count == 1) return m_recording.duration();
    return m_recording.duration() + TimeSpans::divide(m_recording.duration(), count - 1);
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

void FramePlayback::waitUntil(std::chrono::steady_clock::time_point start, TimeSpan due) {
    const auto dueAt = start + std::chrono::duration_cast<std::chrono::steady_clock::duration>(due);
    std::unique_lock lock(m_gate);
    if (dueAt > std::chrono::steady_clock::now())
        m_wake.wait_until(lock, dueAt, [this] { return m_cancelRequested; });
    if (m_cancelRequested) throw Cancelled{};
}

void FramePlayback::notify(const std::function<void()>& callback) {
    m_callbackThreadId.store(std::this_thread::get_id());
    try {
        callback();
    } catch (...) {
        m_callbackThreadId.store(std::thread::id());
        throw;
    }
    m_callbackThreadId.store(std::thread::id());
}

void FramePlayback::playbackLoop(FrameCallback onFrame, MalformedCallback onMalformed,
                                 double playbackSpeed, bool shouldLoop) {
    std::exception_ptr failure;
    try {
        const int count = m_recording.count();
        if (count > 0) {
            const auto timestamps = m_recording.timestamps();
            const auto malformedFrames = m_recording.malformedFrames();
            const TimeSpan first = timestamps[0];
            const TimeSpan pass = passLength();

            // Every frame is scheduled against this one clock, so late frames don't push the
            // rest of the recording back.
            const auto start = std::chrono::steady_clock::now();

            for (int passIndex = 0;; passIndex++) {
                const TimeSpan passOffset = TimeSpans::multiply(pass, passIndex);
                std::size_t nextMalformed = 0;

                // One step past the last frame, for the notes recorded after it.
                for (int i = 0; i <= count; i++) {
                    // The notes recorded between the previous frame and this one. Only the
                    // reader's are replayed: a frame that arrived and was then rejected is in
                    // the recording itself, and is rejected again when it is delivered below.
                    for (; onMalformed && nextMalformed < malformedFrames.size(); nextMalformed++) {
                        const auto& malformed = malformedFrames[nextMalformed];
                        if (malformed.frameIndex > i) break;
                        if (dynamic_cast<const Exceptions::FrameEntryMalformedException*>(
                                malformed.exception.get()))
                            continue;

                        const TimeSpan malformedTimestamp = malformed.timestamp + passOffset;
                        waitUntil(start,
                                  TimeSpans::divide(malformedTimestamp - first, playbackSpeed));
                        notify([&] { onMalformed(*malformed.exception, malformedTimestamp); });
                    }

                    if (i == count) break;

                    const TimeSpan timestamp = timestamps[static_cast<std::size_t>(i)] + passOffset;
                    waitUntil(start, TimeSpans::divide(timestamp - first, playbackSpeed));

                    try {
                        notify([&] {
                            onFrame(m_recording.frames()[static_cast<std::size_t>(i)], timestamp);
                        });
                    } catch (const Exceptions::FrameMalformedException& ex) {
                        if (!onMalformed) throw;
                        notify([&] { onMalformed(ex, timestamp); });
                    }
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
