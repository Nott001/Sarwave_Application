#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <exception>
#include <functional>
#include <mutex>
#include <optional>
#include <stop_token>
#include <thread>

#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"
#include "MMWave/Porting/Context.hpp"
#include "MMWave/Streaming/Frame.hpp"
#include "MMWave/Streaming/FrameReader.hpp"

namespace Sarwave::Core::MMWave::Streaming {

/// Reads frames from the data port continuously and hands each one to a callback.
///
/// Two threads, joined by a bounded queue (docs/pi_frame_corruption.md, "a reader thread and a
/// worker thread"):
///
///   reader: read frame -> timestamp it -> queue it -> read the next frame at once
///   worker: take from the queue -> frame callback (print, record, IMU, display) -> dispose
///
/// The reader never waits on the display, the SD card or the terminal, so the kernel's serial
/// buffer is always drained; on a Raspberry Pi a slow callback no longer turns into lost bytes
/// and damaged frames. Frames are stamped when they arrive, not when they are processed, which
/// keeps them lined up with the IMU even when the worker falls behind. Damaged packets the
/// reader skips go through the same queue, so frames and errors reach the callbacks in order,
/// on the one worker thread.
///
/// If the worker falls so far behind that the queue fills (queueCapacity frames), the oldest
/// queued frame is dropped and reported through onDropped -- "processing too slow", kept apart
/// from "corrupted" so the two problems can't be confused.
class ConstantCapture {
   public:
    /// The frame is disposed right after this callback returns; do not retain it or its bytes
    /// beyond the call. Runs on the worker thread.
    ///
    /// Let a FrameMalformedException for this frame leave the callback (rethrow it if it was
    /// caught): the capture then hands it to the MalformedCallback and carries on. Any other
    /// exception ends the capture.
    ///
    /// timestamp: time since the capture began, taken when the frame arrived.
    using FrameCallback = std::function<void(const Frame& frame, TimeSpan timestamp)>;

    /// Called in place of the frame callback's normal return when a frame is malformed: either
    /// the reader skipped a damaged packet, in which case no frame was delivered for it, or the
    /// frame callback threw a FrameMalformedException for the frame it was given.
    ///
    /// timestamp: on the same clock as the frame callback's. For a frame the callback rejected,
    /// the same timestamp that frame was delivered with.
    using MalformedCallback = std::function<void(
        const Exceptions::FrameMalformedException& exception, TimeSpan timestamp)>;

    /// `count` frames were read intact but dropped unprocessed because the queue was full.
    /// Called on the worker thread just before the next item it does process; `timestamp` is
    /// that item's.
    using DroppedCallback = std::function<void(int count, TimeSpan timestamp)>;

    /// Called once when a capture ends -- stopConstantCapture() was called, the data port
    /// closed, or something failed -- with the exception that ended it (null if none). Runs on
    /// whichever capture thread finished last, after isCapturing() has become false.
    using CompletedCallback = std::function<void(std::exception_ptr error)>;

    static constexpr std::size_t DefaultQueueCapacity = 32;

    explicit ConstantCapture(Porting::Context& ctx);
    ~ConstantCapture();

    ConstantCapture(const ConstantCapture&) = delete;
    ConstantCapture& operator=(const ConstantCapture&) = delete;

    /// Frames the reader may queue ahead of the worker: 32 is about 2 s at 55 ms a frame.
    /// Read when the capture begins.
    std::size_t queueCapacity = DefaultQueueCapacity;

    /// Real-time priority (1 to 99) for the reader thread, so nothing else on the machine can
    /// keep it from draining the port; 0 leaves it a normal thread. Best effort: see
    /// readerIsRealtime(). Read when the capture begins.
    int readerPriority = 0;

    /// Optional; read when the capture begins.
    DroppedCallback onDropped;

    /// Optional; read when the capture begins.
    CompletedCallback onCompleted;

    /// The exception that ended the last capture, if it ended by failing (a failing read or
    /// a callback that threw); null otherwise.
    [[nodiscard]] std::exception_ptr error() const;

    /// Damaged packets the reader has skipped since the capture began.
    [[nodiscard]] int skippedFrames() const {
        return m_skippedFrames.load();
    }

    /// Whether the reader thread got readerPriority. False without the permission for it (see
    /// trySetRealtimePriority), and when readerPriority is 0.
    [[nodiscard]] bool readerIsRealtime() const {
        return m_readerRealtime.load();
    }

    /// Intact frames dropped unprocessed because the worker fell behind, since the capture
    /// began.
    [[nodiscard]] int droppedFrames() const {
        return m_droppedFrames.load();
    }

    /// Time since the capture began; the clock both callbacks' timestamps are read from.
    [[nodiscard]] TimeSpan elapsed() const;

    /// Starts capturing. Throws InvalidOperationException if a capture is still running.
    ///
    /// onMalformed: without one, packets the reader skips go unreported, and a
    /// FrameMalformedException from onNewFrame ends the capture like any other exception.
    void beginConstantCapture(FrameCallback onNewFrame, MalformedCallback onMalformed = {});

    /// Signals the capture to stop and blocks until both threads exit. Frames still queued are
    /// disposed unprocessed.
    ///
    /// waitTimeout bounds how long this call is willing to block; if it passes first, this
    /// throws OperationCanceledException and the capture may still be winding down in the
    /// background. Called from inside a callback, it only signals and returns.
    void stopConstantCapture(std::optional<std::chrono::milliseconds> waitTimeout = std::nullopt);

    [[nodiscard]] bool isCapturing() const;

    /// Blocks until the capture ends (see CompletedCallback). Returns at once when nothing is
    /// capturing. Must not be called from a callback.
    void waitForCompletion();

   private:
    /// A frame, or the error for a packet the reader skipped.
    struct Item {
        std::optional<Frame> frame;
        std::exception_ptr malformed;
        TimeSpan timestamp{};
    };

    void readerLoop(std::stop_token stop);
    void workerLoop(std::stop_token stop);
    void enqueue(Item item);
    void finishThread(std::exception_ptr failure);
    void notifyFrame(const Frame& frame, TimeSpan timestamp);
    void notifyMalformed(const std::exception_ptr& exception, TimeSpan timestamp);
    void notifyDropped(int count, TimeSpan timestamp);
    void joinFinishedThreads();

    FrameReader m_reader;
    mutable std::mutex m_gate;
    std::condition_variable m_finished;

    std::thread m_readerThread;
    std::thread m_workerThread;
    std::stop_source m_stop;
    bool m_running = false;      // guarded by m_gate
    int m_activeThreads = 0;     // guarded by m_gate
    std::exception_ptr m_error;  // guarded by m_gate

    FrameCallback m_onNewFrame;
    MalformedCallback m_onMalformed;
    DroppedCallback m_onDropped;
    CompletedCallback m_onCompleted;
    std::size_t m_capacity = DefaultQueueCapacity;

    std::mutex m_queueGate;
    std::condition_variable_any m_queueChanged;
    std::deque<Item> m_queue;   // guarded by m_queueGate
    bool m_readerDone = false;  // guarded by m_queueGate: nothing more will be queued
    int m_pendingDropped = 0;   // guarded by m_queueGate: dropped since the worker last looked

    std::atomic<std::chrono::steady_clock::rep> m_clockStart{0};
    std::atomic<int> m_skippedFrames{0};
    std::atomic<int> m_droppedFrames{0};
    std::atomic<bool> m_readerRealtime{false};
    std::atomic<std::thread::id> m_callbackThreadId{};
};

}  // namespace Sarwave::Core::MMWave::Streaming
