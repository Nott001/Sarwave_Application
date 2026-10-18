#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>

namespace Sarwave::Core::MMWave {

/// Count, total and longest of a run of durations, for a timing report while the app runs. One
/// thread adds, another takes the figures now and then, without locks: adding costs a few
/// atomic operations, so it can sit in the per-frame path.
///
/// A take() that lands in the middle of an add() may put that one duration in the next report's
/// total but this report's count (or the other way round); for a report that is fine.
class DurationStats {
   public:
    using Duration = std::chrono::nanoseconds;

    struct Figures {
        std::int64_t count = 0;
        Duration total{};
        Duration longest{};

        [[nodiscard]] Duration average() const {
            return count > 0 ? total / count : Duration{};
        }
    };

    void add(Duration duration) {
        const std::int64_t ns = duration.count();
        m_count.fetch_add(1, std::memory_order_relaxed);
        m_total.fetch_add(ns, std::memory_order_relaxed);
        std::int64_t longest = m_longest.load(std::memory_order_relaxed);
        while (ns > longest &&
               !m_longest.compare_exchange_weak(longest, ns, std::memory_order_relaxed)) {
        }
    }

    /// The figures since the last take(), and starts over.
    Figures take() {
        Figures figures;
        figures.count = m_count.exchange(0, std::memory_order_relaxed);
        figures.total = Duration(m_total.exchange(0, std::memory_order_relaxed));
        figures.longest = Duration(m_longest.exchange(0, std::memory_order_relaxed));
        return figures;
    }

   private:
    std::atomic<std::int64_t> m_count{0};
    std::atomic<std::int64_t> m_total{0};
    std::atomic<std::int64_t> m_longest{0};
};

}  // namespace Sarwave::Core::MMWave
