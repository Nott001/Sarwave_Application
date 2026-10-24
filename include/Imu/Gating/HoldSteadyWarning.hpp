#pragma once

#include <optional>

#include "Imu/Gating/MotionGate.hpp"

namespace Sarwave::Core::Imu::Gating {

/// When to tell the operator to hold the device steady. MotionGate refuses frames from the first
/// sign of motion, a knock included, and that stays as it is: those frames are not clean. The
/// warning waits until the gate has refused frames for `delay` without a break, so a knock or a
/// tap on the screen, which the operator can do nothing about once it is over, costs a few
/// frames silently, while moving the device still raises it. It clears with the first frame the
/// gate accepts.
///
/// Call update() once per frame, in order, with that frame's verdict. A timestamp earlier than
/// the previous one (a replay looping) starts it over.
class HoldSteadyWarning {
   public:
    /// How long the gate must have refused frames before the warning shows. A single knock keeps
    /// it refusing for its look-back window (150 ms) and settle time (300 ms), plus up to a frame
    /// of rounding: just under 0.5 s at 55–60 ms frames. Real motion goes on for longer.
    TimeSpan delay = TimeSpans::fromSeconds(0.6);

    /// Whether the warning shows, as of the latest update().
    [[nodiscard]] bool isRaised() const {
        return m_isRaised;
    }

    /// frameTimestamp: the frame's timestamp, as given to MotionGate::evaluate().
    /// state: the gate's verdict on the frame; nullopt without an IMU, which never warns.
    /// Returns whether the warning shows.
    bool update(TimeSpan frameTimestamp, const std::optional<ImuFrameState>& state) {
        if (m_lastFrame && frameTimestamp < *m_lastFrame) reset();
        m_lastFrame = frameTimestamp;

        if (!state || !state->holdSteady())
            m_unsteadySince.reset();
        else if (!m_unsteadySince)
            m_unsteadySince = frameTimestamp;

        m_isRaised = m_unsteadySince && frameTimestamp - *m_unsteadySince >= delay;
        return m_isRaised;
    }

    /// Forgets the frames seen so far; the warning is cleared.
    void reset() {
        m_unsteadySince.reset();
        m_lastFrame.reset();
        m_isRaised = false;
    }

   private:
    std::optional<TimeSpan> m_unsteadySince;
    std::optional<TimeSpan> m_lastFrame;
    bool m_isRaised = false;
};

}  // namespace Sarwave::Core::Imu::Gating
