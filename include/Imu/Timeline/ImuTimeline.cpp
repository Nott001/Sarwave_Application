#include "Imu/Timeline/ImuTimeline.hpp"

#include <algorithm>
#include <bit>
#include <cmath>

#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::Core::Imu::Timeline {

ImuTimeline::ImuTimeline(int capacity) {
    if (capacity < 2) {
        throw ArgumentOutOfRangeException(
            "capacity",
            "capacity ('" + std::to_string(capacity) + "') must be greater than or equal to '2'.");
    }
    const auto size = std::bit_ceil(static_cast<unsigned>(capacity));
    m_states.resize(size);
    m_mask = static_cast<int>(size) - 1;
}

int ImuTimeline::count() const {
    std::lock_guard lock(m_gate);
    return m_count;
}

void ImuTimeline::append(std::span<const ImuState> states) {
    std::lock_guard lock(m_gate);
    const int size = static_cast<int>(m_states.size());
    for (const ImuState& state : states) {
        if (m_count > 0 && state.timestamp <= at(m_count - 1).timestamp) {
            m_discarded++;
            continue;
        }

        if (m_count == size) {
            m_states[static_cast<std::size_t>(m_head)] = state;
            m_head = (m_head + 1) & m_mask;
        } else {
            m_states[static_cast<std::size_t>((m_head + m_count) & m_mask)] = state;
            m_count++;
        }
    }
}

void ImuTimeline::clear() {
    std::lock_guard lock(m_gate);
    m_head = 0;
    m_count = 0;
}

std::optional<ImuState> ImuTimeline::tryGetLatest() const {
    std::lock_guard lock(m_gate);
    if (m_count == 0) return std::nullopt;
    return at(m_count - 1);
}

std::optional<ImuState> ImuTimeline::tryGetAt(TimeSpan time) const {
    std::lock_guard lock(m_gate);
    int index = lowerBound(time);
    if (index == m_count || at(index).timestamp > time) index--;

    if (index < 0) return std::nullopt;
    return at(index);
}

MotionWindow ImuTimeline::summarize(TimeSpan start, TimeSpan end) const {
    std::lock_guard lock(m_gate);
    const int first = lowerBound(start);

    // Before the window's first sample the last one before the window still counts: a gap is
    // only a gap if no sample was taken there at all.
    TimeSpan previous = first > 0 ? at(first - 1).timestamp : start;

    float peakRateSquared = 0;
    float peakLinearSquared = 0;
    Vector3 rateSum;
    Vector3 linearSum;
    Vector3 upSum;
    TimeSpan maxGap = TimeSpan::zero();
    int n = 0;

    for (int i = first; i < m_count; i++) {
        const ImuState& state = at(i);
        if (state.timestamp > end) break;

        const TimeSpan gap = state.timestamp - previous;
        if (gap > maxGap) maxGap = gap;
        previous = state.timestamp;

        const Vector3 linear = state.linearAcceleration();
        peakRateSquared = std::max(peakRateSquared, state.angularRate.lengthSquared());
        peakLinearSquared = std::max(peakLinearSquared, linear.lengthSquared());
        rateSum += state.angularRate;
        linearSum += linear;
        upSum += state.up;
        n++;
    }

    if (n == 0) return MotionWindow::empty(start, end);

    const auto fn = static_cast<float>(n);
    return {start,
            end,
            n,
            std::sqrt(peakRateSquared),
            std::sqrt(peakLinearSquared),
            (rateSum / fn).length(),
            (linearSum / fn).length(),
            MMWave::normalize(upSum),
            maxGap,
            end - previous};
}

int ImuTimeline::lowerBound(TimeSpan time) const {
    int lo = 0;
    int hi = m_count;
    while (lo < hi) {
        const int mid = static_cast<int>(static_cast<unsigned>(lo + hi) >> 1);
        if (at(mid).timestamp < time)
            lo = mid + 1;
        else
            hi = mid;
    }
    return lo;
}

}  // namespace Sarwave::Core::Imu::Timeline
