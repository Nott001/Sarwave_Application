#pragma once

#include <chrono>
#include <cmath>
#include <cstdint>
#include <ratio>

namespace Sarwave::Core::MMWave {

/// The C# code measures every frame time in System.TimeSpan, whose unit is the 100 ns "tick".
/// Recordings store those ticks on disk, so the C++ port keeps the exact same unit.
using TimeSpan = std::chrono::duration<std::int64_t, std::ratio<1, 10'000'000>>;

namespace TimeSpans {

inline constexpr std::int64_t TicksPerMillisecond = 10'000;
inline constexpr std::int64_t TicksPerSecond = 10'000'000;
inline constexpr std::int64_t TicksPerMinute = 600'000'000;

/// Converts any std::chrono duration to ticks, truncating like TimeSpan does.
template <class Rep, class Period>
constexpr TimeSpan from(std::chrono::duration<Rep, Period> d) {
    return std::chrono::duration_cast<TimeSpan>(d);
}

/// TimeSpan.FromSeconds(seconds).
inline TimeSpan fromSeconds(double seconds) {
    return TimeSpan(static_cast<std::int64_t>(std::nearbyint(seconds * TicksPerSecond)));
}

/// TimeSpan / double: ticks divided, rounded to the nearest tick (ties to even, as Math.Round).
inline TimeSpan divide(TimeSpan span, double divisor) {
    return TimeSpan(static_cast<std::int64_t>(std::nearbyint(span.count() / divisor)));
}

/// TimeSpan * double: ticks multiplied, rounded to the nearest tick (ties to even, as Math.Round).
inline TimeSpan multiply(TimeSpan span, double factor) {
    return TimeSpan(static_cast<std::int64_t>(std::nearbyint(span.count() * factor)));
}

/// TimeSpan / TimeSpan: the ratio of the two tick counts.
inline double ratio(TimeSpan a, TimeSpan b) {
    return static_cast<double>(a.count()) / static_cast<double>(b.count());
}

}  // namespace TimeSpans
}  // namespace Sarwave::Core::MMWave
