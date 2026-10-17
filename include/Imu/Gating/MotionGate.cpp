#include "Imu/Gating/MotionGate.hpp"

namespace Sarwave::Core::Imu::Gating {

std::string toString(MotionGateState state) {
    switch (state) {
        case MotionGateState::Steady:
            return "Steady";
        case MotionGateState::Settling:
            return "Settling";
        case MotionGateState::Moving:
            return "Moving";
        case MotionGateState::Unavailable:
            return "Unavailable";
    }
    return std::to_string(static_cast<int>(state));
}

MotionGate::MotionGate(const Timeline::ImuTimeline& timeline, MotionGateSettings settings)
    : m_timeline(timeline), m_settings(settings) {
}

ImuFrameState MotionGate::evaluate(TimeSpan frameTimestamp) {
    if (m_lastFrame && frameTimestamp < *m_lastFrame) reset();
    m_lastFrame = frameTimestamp;

    const MotionGateSettings& s = m_settings;
    const Timeline::MotionWindow window =
        m_timeline.summarize(frameTimestamp - s.lookBack, frameTimestamp);

    const bool covered = window.sampleCount > 0 && window.maxGap <= s.maxSampleGap &&
                         window.trailingGap <= s.maxImuLatency;

    TimeSpan settleRemaining = TimeSpan::zero();
    MotionGateState state;

    if (!covered) {
        // Whatever happened while the data was missing is unknown, so the device has to settle
        // again once data comes back.
        m_moving = true;
        m_quietSince.reset();
        state = MotionGateState::Unavailable;
    } else if (isMoving(window)) {
        m_moving = true;
        m_quietSince.reset();
        state = MotionGateState::Moving;
    } else {
        if (m_moving) {
            m_moving = false;
            m_quietSince = frameTimestamp;
        }

        const TimeSpan quietFor = m_quietSince ? frameTimestamp - *m_quietSince : TimeSpan::max();
        if (quietFor < s.settleTime) {
            settleRemaining = s.settleTime - quietFor;
            state = MotionGateState::Settling;
        } else {
            m_quietSince.reset();
            state = MotionGateState::Steady;
        }
    }

    const bool accepted = state == MotionGateState::Steady ||
                          (state == MotionGateState::Unavailable && s.acceptWhenUnavailable);

    if (accepted)
        m_acceptedFrames++;
    else
        m_rejectedFrames++;
    m_state = state;

    const Fusion::TiltCorrection tilt = window.sampleCount > 0
                                            ? Fusion::TiltCorrection::fromUp(window.meanUp)
                                            : Fusion::TiltCorrection::none();
    return {state, accepted, window, tilt, settleRemaining};
}

void MotionGate::reset() {
    m_lastFrame.reset();
    m_moving = true;
    m_quietSince.reset();
    m_state = MotionGateState::Unavailable;
}

bool MotionGate::isMoving(const Timeline::MotionWindow& window) const {
    const MotionGateSettings& s = m_settings;
    if (window.peakAngularRate > s.joltAngularRateLimit ||
        window.peakLinearAcceleration > s.joltLinearAccelerationLimit)
        return true;

    return m_moving ? window.meanAngularRate > s.angularRateRelease ||
                          window.meanLinearAcceleration > s.linearAccelerationRelease
                    : window.meanAngularRate > s.angularRateLimit ||
                          window.meanLinearAcceleration > s.linearAccelerationLimit;
}

}  // namespace Sarwave::Core::Imu::Gating
