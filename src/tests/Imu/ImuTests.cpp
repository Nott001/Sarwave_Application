// The C# Sarwave.Core.Imu.Tests (FusionTests, GatingTests, SamplingTests), ported one to one.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iterator>
#include <numbers>
#include <random>
#include <sstream>
#include <tuple>
#include <utility>
#include <vector>

#include "Imu/Device/Icm42688.hpp"
#include "Imu/Fusion/Fusion.hpp"
#include "Imu/Fusion/ImuProcessor.hpp"
#include "Imu/Gating/HoldSteadyWarning.hpp"
#include "Imu/Gating/MotionGate.hpp"
#include "Imu/ImuPose.hpp"
#include "Imu/ImuSession.hpp"
#include "Imu/Recording/ImuRecording.hpp"
#include "Imu/Sampling/FifoClock.hpp"
#include "Imu/Timeline/ImuTimeline.hpp"
#include "MMWave/Common/Exceptions.hpp"
#include "TestFramework.hpp"

using namespace Sarwave::Core;
using namespace Sarwave::Core::Imu;
using Fusion::ImuAxis;
using Fusion::ImuMounting;
using Fusion::RigidMotion;
using Fusion::TiltCorrection;
using Gating::HoldSteadyWarning;
using Gating::ImuFrameState;
using Gating::MotionGate;
using Gating::MotionGateState;
using Timeline::ImuTimeline;

namespace {

constexpr float Tolerance = 1e-4f;

bool close(Vector3 expected, Vector3 actual, float tolerance = Tolerance) {
    return MMWave::distance(expected, actual) < tolerance;
}

/// Assert.Equal(expected, actual, precision): equal when rounded to `digits` decimals.
bool equalTo(double expected, double actual, int digits) {
    const double scale = std::pow(10.0, digits);
    return std::nearbyint(expected * scale) == std::nearbyint(actual * scale);
}

float degreesToRadians(float degrees) {
    return degrees * (std::numbers::pi_v<float> / 180.0f);
}

TimeSpan ms(double milliseconds) {
    return TimeSpans::fromSeconds(milliseconds / 1000.0);
}

const TimeSpan SamplePeriod = ms(5);
const TimeSpan FramePeriod = ms(55);

using Signal = std::function<Vector3(TimeSpan)>;

void fill(ImuTimeline& timeline, TimeSpan from, TimeSpan to, const Signal& angularRate,
          const Signal& linearAcceleration = {}) {
    std::vector<ImuState> states;
    for (TimeSpan t = from; t <= to; t += SamplePeriod) {
        const Vector3 acceleration =
            Vector3::unitZ() + (linearAcceleration ? linearAcceleration(t) : Vector3::zero());
        states.push_back({t, acceleration, angularRate(t), Vector3::unitZ(), 25});
    }
    timeline.append(states);
}

std::vector<MotionGateState> evaluateFrames(MotionGate& gate, TimeSpan from, TimeSpan to) {
    std::vector<MotionGateState> states;
    for (TimeSpan t = from; t <= to; t += FramePeriod) states.push_back(gate.evaluate(t).state);
    return states;
}

double totalSeconds(TimeSpan t) {
    return static_cast<double>(t.count()) / TimeSpans::TicksPerSecond;
}

Vector3 tremor(TimeSpan t, float hertz, Vector3 amplitude) {
    return amplitude *
           std::sin(2 * std::numbers::pi_v<float> * hertz * static_cast<float>(totalSeconds(t)));
}

bool contains(const std::vector<MotionGateState>& states, MotionGateState state) {
    return std::find(states.begin(), states.end(), state) != states.end();
}

/// After the first settle every frame is accepted: the gate never refused one.
void checkSteadyOnceSettled(const std::vector<MotionGateState>& states) {
    CHECK(!contains(states, MotionGateState::Moving));
    const auto firstSteady =
        std::find(states.begin(), states.end(), MotionGateState::Steady) - states.begin();
    CHECK(firstSteady >= 0);
    CHECK(firstSteady <= 6);  // the 300 ms settle from start-up, at 55 ms per frame
    CHECK(std::all_of(states.begin() + firstSteady, states.end(),
                      [](MotionGateState s) { return s == MotionGateState::Steady; }));
}

struct WarnedFrame {
    ImuFrameState state;
    bool warned;
};

std::vector<WarnedFrame> evaluateFramesWarned(MotionGate& gate, HoldSteadyWarning& warning,
                                              TimeSpan from, TimeSpan to, TimeSpan framePeriod) {
    std::vector<WarnedFrame> frames;
    for (TimeSpan t = from; t <= to; t += framePeriod) {
        const ImuFrameState state = gate.evaluate(t);
        frames.push_back({state, warning.update(t, state)});
    }
    return frames;
}

/// Quaternion.CreateFromYawPitchRoll: roll about Z, then pitch about X, then yaw about Y.
Quaternion fromYawPitchRoll(float yaw, float pitch, float roll) {
    return MMWave::concatenate(
        MMWave::concatenate(Quaternion::fromAxisAngle(Vector3::unitZ(), roll),
                            Quaternion::fromAxisAngle(Vector3::unitX(), pitch)),
        Quaternion::fromAxisAngle(Vector3::unitY(), yaw));
}

/// Turns left at 90 °/s for 1 s, held level, sampled every 5 ms; returns the first and last state.
std::pair<ImuState, ImuState> turnLeft90(Fusion::ImuProcessor& processor) {
    const ImuState start =
        processor.process({TimeSpan::zero(), Vector3::unitZ(), Vector3::zero(), 25});
    ImuState state;
    for (int i = 1; i <= 200; i++)
        state = processor.process({SamplePeriod * i, Vector3::unitZ(), {0, 0, 90}, 25});
    return {start, state};
}

}  // namespace

// ---- fusion -----------------------------------------------------------------------------------

TEST_CASE("Mounting.FromAxes maps IMU axes onto radar axes") {
    // IMU X arrow at the scene, Y arrow to the left.
    const auto mounting =
        ImuMounting::fromAxes(ImuAxis::NegativeY, ImuAxis::PositiveX, ImuAxis::PositiveZ);
    CHECK(close(Vector3::unitY(), mounting.toRadar(Vector3::unitX())));   // IMU X is forward
    CHECK(close(-Vector3::unitX(), mounting.toRadar(Vector3::unitY())));  // IMU Y is left
    CHECK(close(Vector3::unitZ(), mounting.toRadar(Vector3::unitZ())));
}

TEST_CASE("Mounting on the side wall: nose up is positive pitch") {
    for (float pitchDegrees : {20.0f, -35.0f}) {
        // The handheld's IMU: on a side wall, chip facing the radar's right, X arrow forward, Y up.
        const auto mounting =
            ImuMounting::fromAxes(ImuAxis::PositiveZ, ImuAxis::PositiveX, ImuAxis::PositiveY);
        const float pitch = degreesToRadians(pitchDegrees);
        const Vector3 reading{std::sin(pitch), std::cos(pitch), 0};  // gravity's pull, IMU axes

        const auto tilt = TiltCorrection::fromUp(mounting.toRadar(reading));
        CHECK(equalTo(pitchDegrees, tilt.pitchDegrees(), 3));
        CHECK(equalTo(0, tilt.rollDegrees(), 3));
    }
}

TEST_CASE("Mounting level calibration keeps the radar offset") {
    const Vector3 offset{0.089f, 0.017f, 0.035f};
    ImuMounting mounting = ImuMounting::identity();
    mounting.radarOffset = offset;
    CHECK((mounting.withLevelCalibration({0.03f, -0.05f, 1}).radarOffset == offset));
}

TEST_CASE("Mounting.FromAxes rejects non-rotations") {
    CHECK_THROWS_AS(
        ImuMounting::fromAxes(ImuAxis::PositiveX, ImuAxis::PositiveY, ImuAxis::NegativeZ),
        ArgumentException);  // left-handed
    CHECK_THROWS_AS(
        ImuMounting::fromAxes(ImuAxis::PositiveX, ImuAxis::NegativeX, ImuAxis::PositiveZ),
        ArgumentException);  // axis used twice
}

TEST_CASE("Mounting level calibration makes the measured up vertical") {
    const Vector3 measuredUp = MMWave::normalize(Vector3{0.03f, -0.05f, 1.0f});
    const auto trimmed = ImuMounting::identity().withLevelCalibration(measuredUp);
    CHECK(close(Vector3::unitZ(), trimmed.toRadar(measuredUp)));
}

TEST_CASE("Tilt: a pitched device levels a boresight point") {
    for (float pitchDegrees : {20.0f, -35.0f}) {
        const float pitch = degreesToRadians(pitchDegrees);
        const Vector3 up{0, std::sin(pitch), std::cos(pitch)};

        const auto tilt = TiltCorrection::fromUp(up);
        CHECK(equalTo(pitchDegrees, tilt.pitchDegrees(), 3));
        CHECK(equalTo(0, tilt.rollDegrees(), 3));

        // 3 m straight along the boresight is 3 m away at that elevation in the room.
        const Vector3 level = tilt.apply({0, 3, 0});
        CHECK(close({0, 3 * std::cos(pitch), 3 * std::sin(pitch)}, level));
    }
}

TEST_CASE("Tilt: right side down is positive roll and lowers right points") {
    const float roll = degreesToRadians(15);
    const auto tilt = TiltCorrection::fromUp({-std::sin(roll), 0, std::cos(roll)});
    CHECK(equalTo(15, tilt.rollDegrees(), 3));
    CHECK(tilt.apply(Vector3::unitX()).z < 0);
    CHECK_EQ(tilt.toString(), "pitch +0.0°, roll +15.0°");
}

TEST_CASE("Tilt without data leaves points alone") {
    const auto tilt = TiltCorrection::fromUp(Vector3::zero());
    CHECK(!tilt.isValid());
    CHECK((tilt.apply({1, 2, 3}) == Vector3{1, 2, 3}));
    CHECK_EQ(tilt.toString(), "no tilt data");
}

TEST_CASE("Processor tracks pitch from the gyroscope while accelerating") {
    // Pitch up at 90 °/s for 0.5 s while the accelerometer reads 2 g (not trusted), so only the
    // gyroscope can carry the tilt to 45°.
    Fusion::ImuProcessor processor(ImuMounting::identity(), {.learnGyroBias = false});
    processor.process({TimeSpan::zero(), Vector3::unitZ(), Vector3::zero(), 25});
    ImuState state;
    for (int i = 1; i <= 100; i++)
        state = processor.process({SamplePeriod * i, Vector3::unitZ() * 2, {90, 0, 0}, 25});

    CHECK(equalTo(45, TiltCorrection::fromUp(state.up).pitchDegrees(), 0));
}

TEST_CASE("Processor learns the gyro bias when set down") {
    const Vector3 bias{0.4f, -0.3f, 0.2f};
    Fusion::ImuProcessor processor(ImuMounting::identity());
    for (int i = 0; i < 200 * 30; i++)
        processor.process({SamplePeriod * i, Vector3::unitZ(), bias, 25});

    CHECK(processor.isStill());
    CHECK(MMWave::distance(bias, processor.gyroBias()) < 0.01f);
}

TEST_CASE("Processor tracks heading from the gyroscope") {
    // Turn left at 90 °/s for 1 s, held level: the boresight ends up along room −X.
    Fusion::ImuProcessor processor(ImuMounting::identity(), {.learnGyroBias = false});
    const auto [start, state] = turnLeft90(processor);

    CHECK(close(-Vector3::unitX(), MMWave::transform(Vector3::unitY(), state.orientation), 1e-3f));
    CHECK(close(Vector3::unitZ(), MMWave::transform(state.up, state.orientation), 1e-3f));
}

TEST_CASE("Processor: turned around the IMU, the radar swings by its offset") {
    // The radar 10 cm to the right of the IMU; turn left by 90° about the IMU, held level.
    // The IMU stays put, and the radar swings from room +X to room +Y.
    ImuMounting mounting = ImuMounting::identity();
    mounting.radarOffset = {0.1f, 0, 0};
    Fusion::ImuProcessor processor(mounting, {.learnGyroBias = false});
    const auto [start, state] = turnLeft90(processor);

    CHECK(close({-0.1f, 0.1f, 0}, state.position - start.position, 1e-3f));
}

TEST_CASE("Processor held still stays put") {
    Fusion::ImuProcessor processor(ImuMounting::identity());
    ImuState state;
    for (int i = 0; i < 400; i++)
        state = processor.process({SamplePeriod * i, Vector3::unitZ(), Vector3::zero(), 25});

    CHECK((state.velocity == Vector3::zero()));
    CHECK(state.position.length() < 1e-3f);
}

TEST_CASE("Processor pushed forward moves forward, then stops") {
    // Still, then 0.3 g forward for 0.3 s, the same back for 0.3 s, then still again: about
    // 26 cm forward in truth, less in the estimate, whose velocity decays.
    Fusion::ImuProcessor processor(ImuMounting::identity(), {.learnGyroBias = false});
    ImuState state;
    for (int i = 0; i < 400; i++) {
        const float push = i < 40 ? 0 : i < 100 ? 0.3f : i < 160 ? -0.3f : 0;
        state = processor.process({SamplePeriod * i, {0, push, 1}, Vector3::zero(), 25});
    }

    CHECK((state.velocity == Vector3::zero()));
    CHECK(state.position.y >= 0.1f);
    CHECK(state.position.y <= 0.27f);
    CHECK(std::abs(state.position.x) < 0.02f);
}

TEST_CASE("Pose.motionSince keeps room points in place") {
    // A point 2 m ahead. The device then steps 1 m to room +X and turns left by 90°: the point
    // is now 1 m ahead of it and 2 m to its right.
    const ImuPose earlier{TimeSpan::zero(), Quaternion::identity(), Vector3::zero()};
    const ImuPose later{ms(1000),
                        Quaternion::fromAxisAngle(Vector3::unitZ(), std::numbers::pi_v<float> / 2),
                        Vector3::unitX()};

    CHECK(close({2, 1, 0}, later.motionSince(earlier).apply({0, 2, 0})));
}

TEST_CASE("Motion.reframed moves stored coordinates like the originals") {
    const RigidMotion motion{fromYawPitchRoll(0.3f, -0.2f, 0.1f), {0.1f, -0.2f, 0.05f}};
    const Quaternion before = Quaternion::fromAxisAngle(Vector3::unitX(), -0.26f);
    const Quaternion after = Quaternion::fromAxisAngle(Vector3::unitY(), 0.1f);
    const Vector3 originBefore{0, 0, 2};
    const Vector3 originAfter{0.5f, 0, 1};
    const Vector3 point{0.4f, 3, -0.5f};

    const Vector3 expected = MMWave::transform(motion.apply(point), after) + originAfter;
    const Vector3 stored = MMWave::transform(point, before) + originBefore;

    CHECK(close(expected, motion.reframed(before, originBefore, after, originAfter).apply(stored)));
}

// ---- gating -----------------------------------------------------------------------------------

TEST_CASE("Gate settles, then accepts a steady device") {
    ImuTimeline timeline;
    fill(timeline, TimeSpan::zero(), ms(2000), [](TimeSpan) { return Vector3::zero(); });
    MotionGate gate(timeline);

    const auto states = evaluateFrames(gate, ms(200), ms(2000));
    CHECK(states.front() == MotionGateState::Settling);
    CHECK(states.back() == MotionGateState::Steady);
    CHECK(!contains(states, MotionGateState::Moving));
}

TEST_CASE("Gate refuses frames while turning and until settled") {
    ImuTimeline timeline;
    // Still, a 20 °/s sweep from 1.0 s to 1.5 s, then still again.
    fill(timeline, TimeSpan::zero(), ms(4000), [](TimeSpan t) {
        return t >= ms(1000) && t < ms(1500) ? Vector3{0, 0, 20} : Vector3::zero();
    });
    MotionGate gate(timeline);

    evaluateFrames(gate, ms(200), ms(990));
    CHECK(gate.state() == MotionGateState::Steady);

    const auto during = gate.evaluate(ms(1200));
    CHECK(during.state == MotionGateState::Moving);
    CHECK(!during.accepted);
    CHECK(during.holdSteady());

    // The window's average drops below the release around 1.62 s; settling then takes 0.3 s.
    const auto settling = gate.evaluate(ms(1800));
    CHECK(settling.state == MotionGateState::Settling);
    CHECK(!settling.accepted);

    const auto after = gate.evaluate(ms(2400));
    CHECK(after.state == MotionGateState::Steady);
    CHECK(after.accepted);
}

TEST_CASE("Gate counts linear acceleration alone as motion") {
    ImuTimeline timeline;
    fill(
        timeline, TimeSpan::zero(), ms(2000), [](TimeSpan) { return Vector3::zero(); },
        [](TimeSpan t) { return t >= ms(1500) ? Vector3{0, 0.2f, 0} : Vector3::zero(); });
    MotionGate gate(timeline);

    evaluateFrames(gate, ms(200), ms(1400));
    CHECK(gate.evaluate(ms(1600)).state == MotionGateState::Moving);
}

TEST_CASE("Gate keeps hand tremor steady") {
    ImuTimeline timeline;
    // A held hand: 8 Hz tremor peaking at 11 °/s, nearly twice the average limit.
    fill(
        timeline, TimeSpan::zero(), ms(3000), [](TimeSpan t) { return tremor(t, 8, {8, 0, 8}); },
        [](TimeSpan t) { return tremor(t, 8, {0.04f, 0.04f, 0}); });
    MotionGate gate(timeline);

    const auto states = evaluateFrames(gate, ms(200), ms(3000));
    CHECK(!contains(states, MotionGateState::Moving));
    CHECK(states.back() == MotionGateState::Steady);
}

TEST_CASE("Gate never refuses tremor across its range") {
    // hertz, peak °/s: from the slow tremor of a tired arm, which cancels least over the window,
    // to physiological tremor, 8–12 Hz, strong for a held device.
    for (const auto [hertz, peakRate] :
         {std::pair{4.0f, 8.0f}, {6.0f, 10.0f}, {8.0f, 12.0f}, {10.0f, 12.0f}, {12.0f, 12.0f}}) {
        ImuTimeline timeline;
        const float axis = peakRate / std::sqrt(2.0f);
        fill(
            timeline, TimeSpan::zero(), ms(4000),
            [=](TimeSpan t) { return tremor(t, hertz, {axis, 0, axis}); },
            [=](TimeSpan t) { return tremor(t, hertz, {0.05f, 0.05f, 0}); });
        MotionGate gate(timeline);

        checkSteadyOnceSettled(evaluateFrames(gate, ms(200), ms(4000)));
    }
}

TEST_CASE("Gate never refuses slow sway with tremor") {
    ImuTimeline timeline;
    // The arm and body sway slowly (0.5 Hz, 3 °/s, 0.03 g) under an 8 Hz tremor at 10 °/s.
    fill(
        timeline, TimeSpan::zero(), ms(6000),
        [](TimeSpan t) { return tremor(t, 0.5f, {3, 0, 0}) + tremor(t, 8, {7, 0, 7}); },
        [](TimeSpan t) {
            return tremor(t, 0.5f, {0, 0.03f, 0}) + tremor(t, 8, {0.05f, 0.05f, 0});
        });
    MotionGate gate(timeline);

    checkSteadyOnceSettled(evaluateFrames(gate, ms(200), ms(6000)));
}

TEST_CASE("Gate never refuses slow drift with tremor") {
    ImuTimeline timeline;
    // The aim creeps at 3 °/s, half the limit, under an 8 Hz tremor at 10 °/s.
    fill(timeline, TimeSpan::zero(), ms(4000),
         [](TimeSpan t) { return Vector3{0, 0, 3} + tremor(t, 8, {7, 0, 7}); });
    MotionGate gate(timeline);

    checkSteadyOnceSettled(evaluateFrames(gate, ms(200), ms(4000)));
}

TEST_CASE("Gate refuses a slow deliberate pan") {
    ImuTimeline timeline;
    // Scanning the room at 8 °/s: slow, but a turn that goes one way, so it does not cancel.
    fill(timeline, TimeSpan::zero(), ms(2000), [](TimeSpan t) {
        return (t >= ms(1000) ? Vector3{0, 0, 8} : Vector3::zero()) + tremor(t, 8, {7, 0, 7});
    });
    MotionGate gate(timeline);

    evaluateFrames(gate, ms(200), ms(990));
    CHECK(gate.state() == MotionGateState::Steady);
    CHECK(gate.evaluate(ms(1300)).state == MotionGateState::Moving);
}

TEST_CASE("Gate counts a short jolt as motion") {
    ImuTimeline timeline;
    // A 20 ms knock at 60 °/s: too short to raise the average, but past the jolt limit.
    fill(timeline, TimeSpan::zero(), ms(2000), [](TimeSpan t) {
        return t >= ms(1500) && t < ms(1520) ? Vector3{60, 0, 0} : Vector3::zero();
    });
    MotionGate gate(timeline);

    evaluateFrames(gate, ms(200), ms(1400));
    CHECK(gate.state() == MotionGateState::Steady);
    CHECK(gate.evaluate(ms(1550)).state == MotionGateState::Moving);
}

TEST_CASE("Warning: a knock is refused but not warned") {
    // frame period (ms), knock phase (ms), a push instead of a turn
    for (const auto& [framePeriodMs, phaseMs, linear] : {std::tuple{55, 0, false},
                                                         {55, 20, false},
                                                         {55, 40, false},
                                                         {60, 10, false},
                                                         {60, 50, false},
                                                         {55, 0, true},
                                                         {55, 30, true},
                                                         {60, 45, true}}) {
        ImuTimeline timeline;
        // A tap on the screen or a bump on the handle, on top of a held hand's tremor: a 20 ms
        // turn at 60 °/s, or a 30 ms push at 0.5 g.
        const TimeSpan knock = ms(1500) + ms(phaseMs);
        const auto during = [knock](TimeSpan t, int length) {
            return t >= knock && t < knock + ms(length);
        };
        fill(
            timeline, TimeSpan::zero(), ms(3000),
            [=, l = linear](TimeSpan t) {
                return tremor(t, 8, {7, 0, 7}) +
                       (!l && during(t, 20) ? Vector3{60, 0, 0} : Vector3::zero());
            },
            [=, l = linear](TimeSpan t) {
                return l && during(t, 30) ? Vector3{0, 0.5f, 0} : Vector3::zero();
            });
        MotionGate gate(timeline);
        HoldSteadyWarning warning;

        const auto frames =
            evaluateFramesWarned(gate, warning, ms(200), ms(3000), ms(framePeriodMs));

        // The knock's frames are still left out.
        CHECK(std::any_of(frames.begin(), frames.end(), [](const WarnedFrame& f) {
            return f.state.state == MotionGateState::Moving;
        }));
        CHECK(std::none_of(frames.begin(), frames.end(),
                           [](const WarnedFrame& f) { return f.warned; }));
        CHECK(frames.back().state.accepted);
    }
}

TEST_CASE("Warning: the start-up settle is not warned") {
    ImuTimeline timeline;
    fill(timeline, TimeSpan::zero(), ms(2000), [](TimeSpan) { return Vector3::zero(); });
    MotionGate gate(timeline);
    HoldSteadyWarning warning;

    const auto frames = evaluateFramesWarned(gate, warning, ms(200), ms(2000), FramePeriod);

    CHECK(frames.front().state.state == MotionGateState::Settling);
    CHECK(
        std::none_of(frames.begin(), frames.end(), [](const WarnedFrame& f) { return f.warned; }));
}

TEST_CASE("Warning: a sweep is warned after the delay and clears when steady") {
    ImuTimeline timeline;
    // A 20 °/s sweep from 1.0 s to 2.0 s.
    fill(timeline, TimeSpan::zero(), ms(4000), [](TimeSpan t) {
        return t >= ms(1000) && t < ms(2000) ? Vector3{0, 0, 20} : Vector3::zero();
    });
    MotionGate gate(timeline);
    HoldSteadyWarning warning;

    const auto frames = evaluateFramesWarned(gate, warning, ms(200), ms(4000), FramePeriod);

    const auto index = [&](auto predicate) {
        return static_cast<int>(std::find_if(frames.begin(), frames.end(), predicate) -
                                frames.begin());
    };
    const int firstRefused =
        index([](const WarnedFrame& f) { return f.state.state == MotionGateState::Moving; });
    const int firstWarned = index([](const WarnedFrame& f) { return f.warned; });
    const TimeSpan delay = (firstWarned - firstRefused) * FramePeriod;
    CHECK(delay >= warning.delay);
    CHECK(delay <= warning.delay + FramePeriod);

    // Warned without a break until the gate accepts again, then cleared at once.
    int lastWarned = -1;
    for (int i = 0; i < static_cast<int>(frames.size()); i++)
        if (frames[static_cast<std::size_t>(i)].warned) lastWarned = i;
    REQUIRE(lastWarned >= firstWarned);
    for (int i = firstWarned; i <= lastWarned; i++)
        CHECK(frames[static_cast<std::size_t>(i)].warned);
    CHECK(!frames[static_cast<std::size_t>(lastWarned)].state.accepted);
    CHECK(frames[static_cast<std::size_t>(lastWarned) + 1].state.accepted);
}

TEST_CASE("Warning without IMU or going backwards is cleared") {
    HoldSteadyWarning warning;
    ImuFrameState moving;
    moving.state = MotionGateState::Moving;
    moving.accepted = false;

    warning.update(ms(1000), moving);
    CHECK(warning.update(ms(2000), moving));

    // A looping replay starts over, so the delay runs again.
    CHECK(!warning.update(ms(500), moving));
    CHECK(!warning.update(ms(3000), std::nullopt));
}

TEST_CASE("Gate gives the pose from the middle of the frame's window") {
    ImuTimeline timeline;
    fill(timeline, TimeSpan::zero(), ms(1000), [](TimeSpan) { return Vector3::zero(); });
    MotionGate gate(timeline);

    const auto state = gate.evaluate(ms(500));
    REQUIRE(state.pose.has_value());
    CHECK(state.pose->timestamp == ms(425));           // 500 ms less half the 150 ms look-back
    CHECK(!gate.evaluate(ms(1200)).pose.has_value());  // the data stopped at 1 s
}

TEST_CASE("Gate without IMU data follows the unavailable policy") {
    for (bool acceptWhenUnavailable : {true, false}) {
        ImuTimeline timeline;
        MotionGate gate(timeline, {.acceptWhenUnavailable = acceptWhenUnavailable});
        const auto state = gate.evaluate(ms(1000));
        CHECK(state.state == MotionGateState::Unavailable);
        CHECK_EQ(state.accepted, acceptWhenUnavailable);
        CHECK(!state.tilt.isValid());
    }
}

TEST_CASE("Gate: a stalled IMU becomes unavailable") {
    ImuTimeline timeline;
    fill(timeline, TimeSpan::zero(), ms(1000), [](TimeSpan) { return Vector3::zero(); });
    MotionGate gate(timeline);

    evaluateFrames(gate, ms(200), ms(990));
    CHECK(gate.state() == MotionGateState::Steady);

    // No samples after 1 s: 100 ms later the data lags too far behind.
    CHECK(gate.evaluate(ms(1100)).state == MotionGateState::Unavailable);
}

TEST_CASE("Gate: a timestamp going backwards starts over") {
    ImuTimeline timeline;
    fill(timeline, TimeSpan::zero(), ms(2000), [](TimeSpan) { return Vector3::zero(); });
    MotionGate gate(timeline);

    evaluateFrames(gate, ms(200), ms(1900));
    CHECK(gate.state() == MotionGateState::Steady);

    // A looping replay: the device has to settle again rather than stay Settling forever.
    CHECK(gate.evaluate(ms(300)).state == MotionGateState::Settling);
    CHECK(gate.evaluate(ms(900)).state == MotionGateState::Steady);
}

TEST_CASE("Timeline ring keeps the newest states and summarizes a window") {
    ImuTimeline timeline(64);
    fill(timeline, TimeSpan::zero(), ms(1000),
         [](TimeSpan t) { return Vector3{static_cast<float>(totalSeconds(t)), 0, 0}; });

    CHECK_EQ(timeline.count(), 64);
    const auto latest = timeline.tryGetLatest();
    REQUIRE(latest.has_value());

    const auto window = timeline.summarize(latest->timestamp - ms(50), latest->timestamp);
    CHECK_EQ(window.sampleCount, 11);
    CHECK(equalTo(totalSeconds(latest->timestamp), window.peakAngularRate, 4));
    CHECK(window.maxGap == SamplePeriod);
    CHECK(window.trailingGap == TimeSpan::zero());

    // Older than anything still held: nothing to summarise.
    CHECK_EQ(timeline.summarize(TimeSpan::zero(), ms(100)).sampleCount, 0);
}

// ---- sampling and recording -------------------------------------------------------------------

TEST_CASE("FifoClock recovers the chip rate and keeps samples even and never late") {
    // A chip 1 % fast (as measured on the bench), sampling at its 200 Hz, read every 10 ms with
    // up to 3 ms of scheduling delay.
    const double chipTicksPerSecond = 946'400;
    const double samplePeriod = 4687.5 / chipTicksPerSecond;
    std::mt19937 random(42);
    std::uniform_real_distribution<double> jitter(0.0, 1.0);
    Sampling::FifoClock clock;

    std::vector<Device::Icm42688Reading> readings(Device::Icm42688::MaxPacketsPerRead);
    std::vector<TimeSpan> times(Device::Icm42688::MaxPacketsPerRead);
    std::vector<std::pair<double, double>> samples;  // true, stamped
    bool neverLate = true;

    const double chipStart = 0.0123;
    int next = 0;
    for (double read = 0.02; read < 60; read += 0.010 + jitter(random) * 0.003) {
        std::size_t n = 0;
        for (; chipStart + next * samplePeriod <= read; next++, n++) {
            const auto tick = static_cast<std::uint16_t>(
                static_cast<std::int64_t>(std::nearbyint(next * 4687.5)) & 0xFFFF);
            readings[n] = {Vector3::unitZ(), Vector3::zero(), 25, tick};
        }

        clock.stamp(std::span(readings).first(n), TimeSpans::fromSeconds(read), times);
        for (std::size_t i = 0; i < n; i++) {
            if (times[i] > TimeSpans::fromSeconds(read)) neverLate = false;
            samples.emplace_back(
                chipStart + static_cast<double>(next - static_cast<int>(n) + static_cast<int>(i)) *
                                samplePeriod,
                totalSeconds(times[i]));
        }
    }

    CHECK(neverLate);
    CHECK(std::abs(clock.ticksPerSecond() - chipTicksPerSecond) <= chipTicksPerSecond * 1e-4);

    // After the rate is learned, stamps sit within a millisecond of the truth.
    double worst = 0;
    for (const auto& [truth, stamped] : samples)
        if (truth > 5) worst = std::max(worst, std::abs(stamped - truth));
    CHECK(worst < 0.001);
}

TEST_CASE("Recording round-trips samples and builds a timeline") {
    const std::vector<ImuSample> written{
        {ms(10), {0.1f, 0.2f, 0.98f}, {1, -2, 3}, 26.5f},
        {ms(15), {0.1f, 0.2f, 0.97f}, {1.5f, -2, 3}, 26.5f},
    };

    std::stringstream stream;
    {
        Recording::ImuRecorder recorder(stream, 200);
        recorder.record(written);
        CHECK_EQ(recorder.sampleCount(), 2);
    }

    // A sample cut short at the end, as if the recorder was killed mid-write.
    stream.write(std::string(10, '\0').data(), 10);
    stream.seekg(0);
    const auto recording = Recording::ImuRecording::load(stream);

    CHECK((std::vector<ImuSample>(recording.samples().begin(), recording.samples().end()) ==
           written));
    CHECK_EQ(recording.sampleRate(), 200.0f);
    CHECK(recording.isTruncated());
    CHECK_EQ(recording.toTimeline(ImuMounting::identity())->count(), 2);
    CHECK_EQ(Recording::ImuRecording::sidecarPath("/data/session.swrec").string(),
             "/data/session.swimu");
}

TEST_CASE("Settings scale factors match the datasheet") {
    const Device::Icm42688Settings settings{.gyroRange = Device::GyroRange::Dps500,
                                            .accelRange = Device::AccelRange::G4};
    CHECK(equalTo(65.536, settings.gyroSensitivity(), 3));
    CHECK_EQ(settings.accelSensitivity(), 8192.0f);
    CHECK(equalTo(
        16.384, Device::Icm42688Settings{.gyroRange = Device::GyroRange::Dps2000}.gyroSensitivity(),
        3));
    CHECK_EQ(settings.rateHz(), 200.0f);
}

TEST_CASE("Opening the IMU without one fails with a reason instead of crashing") {
    // No /dev/spidev99.0 on any machine: tryOpen must report it, as on a PC or a Pi without SPI.
    std::string reason;
    auto session = ImuSession::tryOpen({.device = {.busId = 99}}, reason);
    CHECK(session == nullptr);
    CHECK(!reason.empty());
}

// ---- parity with the C# library ---------------------------------------------------------------

namespace {

std::string runHarness(const std::string& args) {
    const std::string command = std::string(SARWAVE_IMU_HARNESS) + " " + args;
    std::string output;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return output;
    char buffer[4096];
    std::size_t n;
    while ((n = fread(buffer, 1, sizeof buffer, pipe)) > 0) output.append(buffer, n);
    pclose(pipe);
    return output;
}

std::string readFixture(const std::string& name) {
    std::ifstream in(std::string(SARWAVE_TEST_DATA_DIR) + "/imu/" + name, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

}  // namespace

TEST_CASE("parity: fusion, gating and mounting equal the C# IMU library's, to the last bit") {
    const std::string data = std::string(SARWAVE_TEST_DATA_DIR) + "/imu/";
    for (const auto& [args, fixture] :
         {std::pair{"process " + data + "session.swimu 10", "session.process10.txt"},
          std::pair{"track " + data + "session.swimu 10", "session.track10.txt"},
          std::pair{"gate " + data + "session.swimu 55", "session.gate55.txt"},
          std::pair{std::string("mount"), "mount.txt"}}) {
        const std::string expected = readFixture(fixture);
        REQUIRE(!expected.empty());
        CHECK(runHarness(args) == expected);
    }
}
