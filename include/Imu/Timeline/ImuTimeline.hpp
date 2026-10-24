#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <optional>
#include <span>
#include <vector>

#include "Imu/ImuSample.hpp"

namespace Sarwave::Core::Imu::Timeline {

/// What the IMU saw between two instants: how hard the device moved and which way was up.
struct MotionWindow {
    TimeSpan start{};
    TimeSpan end{};

    /// Samples inside the window. Zero means nothing below is meaningful.
    int sampleCount = 0;

    /// Largest angular rate magnitude, °/s.
    float peakAngularRate = 0;

    /// Largest acceleration magnitude with gravity removed, g.
    float peakLinearAcceleration = 0;

    /// Magnitude of the average angular rate vector, °/s: the net turn over the window divided
    /// by its length. Hand tremor swings back and forth and mostly cancels here; a deliberate
    /// turn does not.
    float meanAngularRate = 0;

    /// Magnitude of the average linear acceleration vector, g: the change in velocity over the
    /// window divided by its length. Tremor cancels here too.
    float meanLinearAcceleration = 0;

    /// Average "up" direction in radar axes, normalised.
    Vector3 meanUp;

    /// Longest stretch of the window without a sample, not counting after the last one: data the
    /// IMU lost (FIFO overflow, sampler stalled) or never had (IMU started late).
    TimeSpan maxGap{};

    /// From the last sample to `end`: how far the IMU data lags the radar. A few milliseconds
    /// normally; large when the IMU stopped delivering.
    TimeSpan trailingGap{};

    static MotionWindow empty(TimeSpan start, TimeSpan end) {
        return {start, end, 0, 0, 0, 0, 0, Vector3::zero(), end - start, end - start};
    }
};

/// The most recent processed IMU states, in time order, so each radar frame can be paired with
/// what the IMU measured while that frame was being captured.
///
/// A fixed-size ring: once full, every append drops the oldest states. 4096 states is 20 s at
/// 200 Hz, far more than any frame needs. One thread appends (the sampler) while others
/// summarise (the radar capture); a mutex keeps them apart, and both hold it only for a few
/// microseconds. Nothing is allocated after construction.
class ImuTimeline {
   public:
    /// capacity: rounded up to a power of two. Throws ArgumentOutOfRangeException below 2.
    explicit ImuTimeline(int capacity = 4096);

    [[nodiscard]] int capacity() const {
        return static_cast<int>(m_states.size());
    }

    [[nodiscard]] int count() const;

    /// States refused by append() because they were not newer than the last one.
    [[nodiscard]] std::int64_t discarded() const {
        return m_discarded.load();
    }

    /// Adds states, which must be newer than everything already held; older ones are dropped
    /// and counted.
    void append(std::span<const ImuState> states);

    void clear();

    [[nodiscard]] std::optional<ImuState> tryGetLatest() const;

    /// The latest state taken at or before `time`; nullopt if there is none.
    [[nodiscard]] std::optional<ImuState> tryGetAt(TimeSpan time) const;

    /// Peak and average motion, mean "up" and data coverage over [start, end].
    [[nodiscard]] MotionWindow summarize(TimeSpan start, TimeSpan end) const;

   private:
    [[nodiscard]] int lowerBound(TimeSpan time) const;

    [[nodiscard]] const ImuState& at(int logicalIndex) const {
        return m_states[static_cast<std::size_t>((m_head + logicalIndex) & m_mask)];
    }

    std::vector<ImuState> m_states;
    int m_mask;
    mutable std::mutex m_gate;

    int m_head = 0;  // physical index of the oldest state
    int m_count = 0;
    std::atomic<std::int64_t> m_discarded{0};
};

}  // namespace Sarwave::Core::Imu::Timeline
