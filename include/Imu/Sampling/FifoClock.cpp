#include "Imu/Sampling/FifoClock.hpp"

#include <algorithm>
#include <cmath>

namespace Sarwave::Core::Imu::Sampling {

void FifoClock::stamp(std::span<const Device::Icm42688Reading> readings, TimeSpan readTime,
                      std::span<TimeSpan> times) {
    if (readings.empty()) return;

    const bool first = !m_started;
    for (std::size_t i = 0; i < readings.size(); i++) {
        const std::uint16_t raw = readings[i].timestamp;
        // Packets are one output period apart, far less than the 16-bit counter's wrap, so the
        // difference modulo 2^16 is always the true one.
        if (first && i == 0)
            m_unwrapped = 0;
        else
            m_unwrapped += static_cast<std::uint16_t>(raw - m_lastRaw);
        m_lastRaw = raw;
        m_ticks[i] = m_unwrapped;
    }

    const std::int64_t newest = m_ticks[readings.size() - 1];
    const auto host = static_cast<double>(readTime.count());
    const double hostPerTick = m_hostPerTick.load();

    if (first) {
        m_started = true;
        m_offset = host - static_cast<double>(newest) * hostPerTick;
        m_baseTicks = newest;
        m_baseHost = host;
        m_sw = m_sx = m_sy = m_sxx = m_sxy = 0;
    } else {
        updateRate(newest, host);

        const double residual =
            host - (m_offset + static_cast<double>(newest) * m_hostPerTick.load());
        m_offset += std::min(residual, Leak * std::max(host - m_lastHost, 0.0));
    }

    m_lastHost = host;

    const double rate = m_hostPerTick.load();
    for (std::size_t i = 0; i < readings.size(); i++) {
        // Math.Round: to nearest, ties to even.
        auto t = static_cast<std::int64_t>(
            std::nearbyint(m_offset + static_cast<double>(m_ticks[i]) * rate));
        // Strictly increasing, even when the line drops.
        if (t <= m_lastEmitted) t = m_lastEmitted + 1;
        m_lastEmitted = t;
        times[i] = TimeSpan(t);
    }
}

void FifoClock::updateRate(std::int64_t newest, double host) {
    const auto x = static_cast<double>(newest - m_baseTicks);
    const double y = host - m_baseHost;

    m_sw = m_sw * Forget + 1;
    m_sx = m_sx * Forget + x;
    m_sy = m_sy * Forget + y;
    m_sxx = m_sxx * Forget + x * x;
    m_sxy = m_sxy * Forget + x * y;

    if (m_sw < MinFitWeight || y < MinFitSpan) return;

    const double meanX = m_sx / m_sw;
    const double variance = m_sxx / m_sw - meanX * meanX;
    if (!(variance > 0)) return;

    const double slope = (m_sxy / m_sw - meanX * (m_sy / m_sw)) / variance;
    if (std::abs(slope / NominalHostPerTick - 1) > MaxRateError) return;

    // Keep the line where it was at the newest packet, so the new rate does not make the
    // timeline jump.
    const double predicted = m_offset + static_cast<double>(newest) * m_hostPerTick.load();
    m_hostPerTick.store(slope);
    m_offset = predicted - static_cast<double>(newest) * slope;
}

}  // namespace Sarwave::Core::Imu::Sampling
