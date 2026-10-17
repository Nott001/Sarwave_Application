#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "Imu/Fusion/Fusion.hpp"
#include "Imu/Timeline/ImuTimeline.hpp"

namespace Sarwave::Core::Imu::Gating {

/// Thresholds for MotionGate. The defaults are starting points for a handheld device and are
/// meant to be tuned against recorded sessions: record with the IMU, then replay with different
/// settings and compare which frames each one keeps.
struct MotionGateSettings {
    /// How far back from a frame's arrival its IMU window reaches. A frame reaches the Pi well
    /// after its chirps were sent: one frame period of processing on the radar plus the UART
    /// transfer. 150 ms covers that for frame periods up to about 60 ms, as in the configs here.
    TimeSpan lookBack = TimeSpans::fromSeconds(0.150);

    /// Average angular rate over the look-back window that marks the device as moving, °/s (see
    /// MotionWindow::meanAngularRate). Turning at 6 °/s with the radar about 0.4 m from the
    /// wrist moves it at roughly 0.04 m/s, about where the hand's motion starts to show as
    /// Doppler on static surroundings. The average, not the peak, is compared: hand tremor
    /// (roughly 4–12 Hz) reaches well past this for a few milliseconds at a time but swings back
    /// and forth, so it barely moves the device and mostly cancels out of the average.
    float angularRateLimit = 6.0f;

    /// Once moving, the average angular rate must drop below this before the device counts as
    /// still again.
    float angularRateRelease = 4.0f;

    /// Average acceleration (gravity removed) that marks the device as moving, g. Catches
    /// walking and lunges that barely rotate it.
    float linearAccelerationLimit = 0.08f;

    /// Once moving, the average acceleration must drop below this before the device counts as
    /// still again.
    float linearAccelerationRelease = 0.05f;

    /// Peak angular rate that marks the device as moving on its own, °/s: a jolt or a quick
    /// flick too short to raise the average. Set well above what tremor reaches.
    float joltAngularRateLimit = 40.0f;

    /// Peak acceleration (gravity removed) that marks the device as moving on its own, g. A bump
    /// or a knock.
    float joltLinearAccelerationLimit = 0.3f;

    /// How long the device must stay still after moving before frames are used again, so the
    /// hand has settled. (The thesis's 2-second pause before each recording window is part of
    /// the collection protocol, on top of this.)
    TimeSpan settleTime = TimeSpans::fromSeconds(0.300);

    /// A hole in the IMU data longer than this inside a frame's window makes the frame's motion
    /// unknown.
    TimeSpan maxSampleGap = TimeSpans::fromSeconds(0.040);

    /// IMU data lagging the frame by more than this makes the frame's motion unknown.
    TimeSpan maxImuLatency = TimeSpans::fromSeconds(0.080);

    /// What to do with frames whose motion is unknown (no IMU, or it stopped delivering).
    /// Accepting them keeps the device detecting without motion gating, so a failed IMU cannot
    /// silence it; refusing them is the right choice when collecting training data.
    bool acceptWhenUnavailable = true;
};

enum class MotionGateState : std::uint8_t {
    /// Held still and settled: the frame can be classified.
    Steady,
    /// Still again after moving, but not for long enough yet.
    Settling,
    /// Moving beyond the thresholds.
    Moving,
    /// No usable IMU data for this frame.
    Unavailable,
};

/// Enum.ToString().
std::string toString(MotionGateState state);

/// Everything the IMU says about one radar frame.
struct ImuFrameState {
    /// The gate's verdict on the device's motion.
    MotionGateState state = MotionGateState::Unavailable;
    /// Whether the frame should go on to classification.
    bool accepted = false;
    /// The IMU data the verdict was based on.
    Timeline::MotionWindow window;
    /// Levels the frame's points; TiltCorrection::none() without IMU data.
    Fusion::TiltCorrection tilt;
    /// While Settling, how much longer it lasts.
    TimeSpan settleRemaining{};

    /// The operator should be told to hold the device steady.
    [[nodiscard]] bool holdSteady() const {
        return state == MotionGateState::Moving || state == MotionGateState::Settling;
    }
};

/// The thesis's motion gating and tilt compensation, one radar frame at a time: pairs each frame
/// with the IMU data from while it was captured, decides whether the device was steady enough
/// for the frame to be classified, and gives the rotation that levels its points.
///
/// Moving a handheld radar gives static walls and furniture an apparent Doppler velocity that
/// looks like human micro-motion; frames from such moments are refused. Motion is judged on the
/// average over the frame's window, so hand tremor, which swings back and forth, does not count;
/// a jolt counts on its peak alone. The average thresholds have hysteresis, and after any motion
/// the device must stay still for MotionGateSettings::settleTime before frames are accepted
/// again.
///
/// Call evaluate() once per frame, in order, from one thread (the capture callback). A
/// timestamp earlier than the previous one (a replay looping) starts it over.
class MotionGate {
   public:
    /// The timeline must outlive the gate.
    explicit MotionGate(const Timeline::ImuTimeline& timeline, MotionGateSettings settings = {});

    [[nodiscard]] const MotionGateSettings& settings() const {
        return m_settings;
    }

    [[nodiscard]] std::int64_t acceptedFrames() const {
        return m_acceptedFrames;
    }

    [[nodiscard]] std::int64_t rejectedFrames() const {
        return m_rejectedFrames;
    }

    /// The state given for the latest frame.
    [[nodiscard]] MotionGateState state() const {
        return m_state;
    }

    /// frameTimestamp: when the frame arrived, on the clock the IMU samples are stamped with.
    ImuFrameState evaluate(TimeSpan frameTimestamp);

    /// Forgets all motion history, as if no frame had been evaluated. The counters are kept.
    void reset();

   private:
    [[nodiscard]] bool isMoving(const Timeline::MotionWindow& window) const;

    const Timeline::ImuTimeline& m_timeline;
    MotionGateSettings m_settings;
    std::optional<TimeSpan> m_lastFrame;
    bool m_moving = true;  // nothing known yet: the first steady frames have to settle
    std::optional<TimeSpan> m_quietSince;
    std::int64_t m_acceptedFrames = 0;
    std::int64_t m_rejectedFrames = 0;
    MotionGateState m_state = MotionGateState::Unavailable;
};

}  // namespace Sarwave::Core::Imu::Gating
