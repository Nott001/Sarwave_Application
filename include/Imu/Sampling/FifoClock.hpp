#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <span>

#include "Imu/Device/Icm42688.hpp"

namespace Sarwave::Core::Imu::Sampling {

/// Puts FIFO packets on the host clock. The chip's 16-bit timestamps give exact spacing between
/// packets; the host's clock, read just before each FIFO read, says when the newest of them was
/// taken at the latest. The two are joined by a line, host = offset + ticks × rate:
///
///   - Rate: the timestamp tick is nominally 1/937 500 s (the datasheet's "1 µs" is off by 15/16
///     relative to the output data rate, measured on the part), and the chip's oscillator adds
///     about 1 % on top. The rate is a least-squares fit of host time against chip ticks over
///     the last half-minute or so of reads, which averages the read-time jitter away.
///   - Offset: a packet cannot have been taken after the read that found it, so the line is kept
///     under every read. It drops at once to stay under a read, and otherwise rises only slowly
///     (Leak), so the newest packet's age (anywhere up to one output period) and scheduling
///     delays, which only ever make reads late, do not turn into jitter.
///
/// The result is sample spacing as even as the chip's and sample times within about a
/// millisecond, well inside what the motion window (150 ms) needs.
class FifoClock {
   public:
    /// Chip timestamp ticks per second, as currently measured. Safe from any thread.
    [[nodiscard]] double ticksPerSecond() const {
        return static_cast<double>(TimeSpans::TicksPerSecond) / m_hostPerTick.load();
    }

    /// After lost packets (FIFO overflow) the chip timestamps no longer follow on. The line is
    /// found again from the next read; the measured rate is kept.
    void restart() {
        m_started = false;
    }

    /// readings: packets from one FIFO read, oldest first.
    /// readTime: host time taken just before the FIFO count was read.
    /// times: receives each packet's host time; at least as long as `readings`.
    void stamp(std::span<const Device::Icm42688Reading> readings, TimeSpan readTime,
               std::span<TimeSpan> times);

   private:
    static constexpr double NominalTicksPerSecond = 937'500;

    /// The rate is only taken from the fit once it has this many reads...
    static constexpr double MinFitWeight = 50;

    /// ...spread over at least this many host ticks (1 s).
    static constexpr double MinFitSpan = static_cast<double>(TimeSpans::TicksPerSecond);

    /// A fit further than this from nominal is a bad fit, not a strange chip.
    static constexpr double MaxRateError = 0.05;

    /// Per read: the fit forgets with a time constant of 3000 reads, 30 s at 100 polls a second.
    static constexpr double Forget = 1 - 1.0 / 3000;

    /// How fast the offset may rise, host ticks per host tick: 1 ms per second.
    static constexpr double Leak = 1e-3;

    static constexpr double NominalHostPerTick =
        static_cast<double>(TimeSpans::TicksPerSecond) / NominalTicksPerSecond;

    void updateRate(std::int64_t newest, double host);

    std::array<std::int64_t, Device::Icm42688::MaxPacketsPerRead> m_ticks{};

    std::atomic<double> m_hostPerTick{NominalHostPerTick};
    bool m_started = false;
    std::uint16_t m_lastRaw = 0;
    std::int64_t m_unwrapped = 0;
    double m_offset = 0;
    double m_lastHost = 0;
    std::int64_t m_lastEmitted = INT64_MIN;

    // Weighted sums for the fit, relative to the first read so the numbers stay small.
    std::int64_t m_baseTicks = 0;
    double m_baseHost = 0;
    double m_sw = 0, m_sx = 0, m_sy = 0, m_sxx = 0, m_sxy = 0;
};

}  // namespace Sarwave::Core::Imu::Sampling
