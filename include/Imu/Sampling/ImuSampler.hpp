#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <exception>
#include <functional>
#include <mutex>
#include <span>
#include <thread>

#include "Imu/Device/Icm42688.hpp"
#include "Imu/Fusion/ImuProcessor.hpp"
#include "Imu/Sampling/FifoClock.hpp"
#include "Imu/Timeline/ImuTimeline.hpp"

namespace Sarwave::Core::Imu::Sampling {

/// Keeps the IMU flowing into an ImuTimeline: a dedicated thread empties the FIFO every
/// pollInterval, stamps each packet on the caller's clock, processes it and appends the batch.
/// Everything is preallocated; a steady-state poll allocates nothing.
///
/// Its own thread, so a busy radar capture or GUI cannot delay it. The FIFO absorbs whatever
/// delays remain: it holds 640 ms at 200 Hz.
class ImuSampler {
   public:
    /// Receives each batch of raw samples right after it reaches the timeline. Called on the
    /// sampler thread; the span is only valid during the call.
    using BatchCallback = std::function<void(std::span<const ImuSample> samples)>;

    /// The clock samples are stamped with.
    using Clock = std::function<TimeSpan()>;

    /// Called once on the sampler thread when sampling ends, with the exception that ended it
    /// (null when stopped).
    using CompletedCallback = std::function<void(std::exception_ptr error)>;

    ImuSampler(Device::Icm42688& device, Fusion::ImuProcessor& processor,
               Timeline::ImuTimeline& timeline);
    ~ImuSampler();
    ImuSampler(const ImuSampler&) = delete;
    ImuSampler& operator=(const ImuSampler&) = delete;

    /// How often the FIFO is emptied. Read when sampling starts.
    TimeSpan pollInterval = TimeSpans::fromSeconds(0.010);

    /// Optional; read when sampling starts.
    CompletedCallback onCompleted;

    [[nodiscard]] bool isRunning() const;

    /// Blocks until sampling ends. Returns at once when it is not running.
    void waitForCompletion();

    /// The exception that ended sampling, if it ended by failing.
    [[nodiscard]] std::exception_ptr error() const;

    /// The exception the batch callback threw, if it did. The callback is dropped and sampling
    /// carries on, so a full disk stops the IMU recording but not the motion gating.
    [[nodiscard]] std::exception_ptr callbackError() const;

    /// Samples delivered since sampling began.
    [[nodiscard]] std::int64_t sampleCount() const {
        return m_samples.load();
    }

    /// Times the FIFO overflowed and samples were lost.
    [[nodiscard]] int overflows() const {
        return m_overflows.load();
    }

    /// The chip's timestamp rate as measured against the host clock, ticks per second.
    [[nodiscard]] double measuredTimestampRate() const {
        return m_clock.ticksPerSecond();
    }

    /// clock: must be the one the radar frames are stamped with, e.g.
    /// `[&] { return capture.elapsed(); }`, and already running.
    /// onBatch: optional, e.g. an ImuRecorder's record.
    /// Throws InvalidOperationException if already running.
    void start(Clock clock, BatchCallback onBatch = {});

    /// Stops sampling and waits for the thread to finish its current poll.
    void stop();

   private:
    /// Consecutive failed reads tolerated (a loose wire's glitch) before the sampler gives up.
    static constexpr int MaxConsecutiveFailures = 20;

    void run(Clock clock, BatchCallback onBatch, CompletedCallback onCompleted,
             std::chrono::steady_clock::duration period);
    bool waitForNextPoll(std::chrono::steady_clock::time_point& next,
                         std::chrono::steady_clock::duration period);

    Device::Icm42688& m_device;
    Fusion::ImuProcessor& m_processor;
    Timeline::ImuTimeline& m_timeline;
    FifoClock m_clock;

    mutable std::mutex m_gate;
    std::condition_variable m_wake;
    std::thread m_thread;
    bool m_running = false;              // guarded by m_gate
    bool m_stopRequested = false;        // guarded by m_gate
    std::exception_ptr m_error;          // guarded by m_gate
    std::exception_ptr m_callbackError;  // guarded by m_gate
    std::atomic<std::int64_t> m_samples{0};
    std::atomic<int> m_overflows{0};
};

}  // namespace Sarwave::Core::Imu::Sampling
