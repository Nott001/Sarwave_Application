#include "Imu/Sampling/ImuSampler.hpp"

#include <array>

#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Common/ThreadPriority.hpp"

namespace Sarwave::Core::Imu::Sampling {

ImuSampler::ImuSampler(Device::Icm42688& device, Fusion::ImuProcessor& processor,
                       Timeline::ImuTimeline& timeline)
    : m_device(device), m_processor(processor), m_timeline(timeline) {
}

ImuSampler::~ImuSampler() {
    stop();
    if (m_thread.joinable()) m_thread.detach();  // only when destroyed on its own thread
}

bool ImuSampler::isRunning() const {
    std::lock_guard lock(m_gate);
    return m_running;
}

void ImuSampler::waitForCompletion() {
    std::unique_lock lock(m_gate);
    m_wake.wait(lock, [this] { return !m_running; });
}

std::exception_ptr ImuSampler::error() const {
    std::lock_guard lock(m_gate);
    return m_error;
}

std::exception_ptr ImuSampler::callbackError() const {
    std::lock_guard lock(m_gate);
    return m_callbackError;
}

void ImuSampler::start(Clock clock, BatchCallback onBatch) {
    if (!clock) throw ArgumentException("Value cannot be null.", "clock");

    std::lock_guard lock(m_gate);
    if (m_running) throw InvalidOperationException("ImuSampler is already running.");
    if (m_thread.joinable() && m_thread.get_id() != std::this_thread::get_id()) m_thread.join();

    m_stopRequested = false;
    m_error = nullptr;
    m_callbackError = nullptr;
    m_clock.restart();
    m_processor.reset();
    m_device.flushFifo();

    m_running = true;
    const auto period =
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(pollInterval);
    m_thread =
        std::thread([this, clock = std::move(clock), onBatch = std::move(onBatch),
                     completed = onCompleted, period] { run(clock, onBatch, completed, period); });
    m_realtime = priority > 0 && MMWave::trySetRealtimePriority(m_thread, priority);
}

void ImuSampler::stop() {
    {
        std::lock_guard lock(m_gate);
        m_stopRequested = true;
    }
    m_wake.notify_all();

    if (m_thread.joinable() && m_thread.get_id() != std::this_thread::get_id()) m_thread.join();
}

/// Sleeps until the next poll is due. False when a stop was requested meanwhile.
bool ImuSampler::waitForNextPoll(std::chrono::steady_clock::time_point& next,
                                 std::chrono::steady_clock::duration period) {
    next += period;
    const auto now = std::chrono::steady_clock::now();

    // Fell more than a poll behind (debugger, heavy load): start the schedule over instead of
    // firing a burst of polls to catch up. The FIFO kept the samples meanwhile.
    if (now - next > period) next = now;

    std::unique_lock lock(m_gate);
    m_wake.wait_until(lock, next, [this] { return m_stopRequested; });
    return !m_stopRequested;
}

void ImuSampler::run(Clock clock, BatchCallback onBatch, CompletedCallback onCompleted,
                     std::chrono::steady_clock::duration period) {
    std::array<Device::Icm42688Reading, Device::Icm42688::MaxPacketsPerRead> readings{};
    std::array<TimeSpan, Device::Icm42688::MaxPacketsPerRead> times{};
    std::array<ImuSample, Device::Icm42688::MaxPacketsPerRead> samples{};
    std::array<ImuState, Device::Icm42688::MaxPacketsPerRead> states{};
    int failures = 0;
    std::exception_ptr failure;

    try {
        auto next = std::chrono::steady_clock::now();
        while (true) {
            {
                std::lock_guard lock(m_gate);
                if (m_stopRequested) break;
            }

            int count = 0;
            bool overflowed = false;
            const TimeSpan readTime = clock();
            try {
                count = m_device.readFifo(readings, overflowed);
                failures = 0;
            } catch (const IOException&) {
                if (++failures >= MaxConsecutiveFailures) throw;
                if (!waitForNextPoll(next, period)) break;
                continue;
            }

            if (overflowed) {
                m_overflows++;
                m_clock.restart();
            }

            if (count > 0) {
                const auto n = static_cast<std::size_t>(count);
                m_clock.stamp(std::span(readings).first(n), readTime, times);
                for (std::size_t i = 0; i < n; i++) {
                    const auto& r = readings[i];
                    samples[i] = {times[i], r.acceleration, r.angularRate, r.temperature};
                    states[i] = m_processor.process(samples[i]);
                }

                m_timeline.append(std::span(states).first(n));
                m_samples += count;

                if (onBatch) {
                    try {
                        onBatch(std::span<const ImuSample>(samples).first(n));
                    } catch (...) {
                        std::lock_guard lock(m_gate);
                        m_callbackError = std::current_exception();
                        onBatch = nullptr;
                    }
                }
            }

            if (!waitForNextPoll(next, period)) break;
        }
    } catch (...) {
        failure = std::current_exception();
    }

    {
        std::lock_guard lock(m_gate);
        m_error = failure;
        m_running = false;
    }
    m_wake.notify_all();

    if (onCompleted) {
        try {
            onCompleted(failure);
        } catch (...) {
            // Nothing is left to report it to.
        }
    }
}

}  // namespace Sarwave::Core::Imu::Sampling
