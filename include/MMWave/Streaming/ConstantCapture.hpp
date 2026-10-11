#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <mutex>
#include <optional>
#include <stop_token>
#include <thread>

#include "MMWave/Porting/Context.hpp"
#include "MMWave/Streaming/Frame.hpp"
#include "MMWave/Streaming/FrameReader.hpp"

namespace Sarwave::Core::MMWave::Streaming {

/// Reads frames from the data port continuously on a background thread and hands each one to a
/// callback.
class ConstantCapture {
   public:
    /// The frame is disposed right after this callback returns; do not retain it or its
    /// bytes beyond the call. Runs on the capture thread.
    using FrameCallback = std::function<void(const Frame& frame)>;

    explicit ConstantCapture(Porting::Context& ctx);
    ~ConstantCapture();

    ConstantCapture(const ConstantCapture&) = delete;
    ConstantCapture& operator=(const ConstantCapture&) = delete;

    /// The exception that ended the last capture, if it ended by failing (a failing read or
    /// a callback that threw); null otherwise.
    [[nodiscard]] std::exception_ptr error() const;

    /// Starts capturing. Throws InvalidOperationException if a capture is still running.
    void beginConstantCapture(FrameCallback onNewFrame);

    /// Signals the capture loop to stop and blocks until it exits.
    ///
    /// waitTimeout bounds how long this call is willing to block; if it passes first, this
    /// throws OperationCanceledException and the capture may still be winding down in the
    /// background. Called from inside the frame callback, it only signals and returns.
    void stopConstantCapture(std::optional<std::chrono::milliseconds> waitTimeout = std::nullopt);

    [[nodiscard]] bool isCapturing() const;

   private:
    void captureLoop(std::stop_token stop);
    void notifyFrame(const Frame& frame);
    void joinFinishedThread();

    FrameReader m_reader;
    mutable std::mutex m_gate;
    std::condition_variable m_finished;

    std::thread m_thread;
    std::stop_source m_stop;
    bool m_running = false;  // guarded by m_gate
    FrameCallback m_onNewFrame;
    std::atomic<std::thread::id> m_callbackThreadId{};
    std::exception_ptr m_error;  // guarded by m_gate
};

}  // namespace Sarwave::Core::MMWave::Streaming
