#include "Imu/Fusion/ImuProcessor.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace Sarwave::Core::Imu::Fusion {
namespace {

constexpr float FastWindowSeconds = 0.25f;
constexpr float StandardGravity = 9.80665f;  // m/s² per g

float seconds(TimeSpan span) {
    return static_cast<float>(static_cast<double>(span.count()) / TimeSpans::TicksPerSecond);
}

/// Tilts `orientation` to agree with `up`, keeping its heading.
Quaternion level(Quaternion orientation, Vector3 up) {
    const Vector3 predictedUp = MMWave::transform(Vector3::unitZ(), orientation.conjugate());
    const Quaternion correction = TiltMath::shortestArc(predictedUp, up);
    return MMWave::normalize(MMWave::concatenate(correction.conjugate(), orientation));
}

}  // namespace

ImuProcessor::ImuProcessor(ImuMounting mounting, ImuProcessorSettings settings)
    : m_settings(settings),
      m_mounting(mounting),
      m_tiltTimeConstant(seconds(settings.tiltTimeConstant)),
      m_biasTimeConstant(seconds(settings.biasTimeConstant)),
      m_stillTime(seconds(settings.stillTime)),
      m_maxGap(seconds(settings.maxSampleGap)),
      m_velocityTimeConstant(seconds(settings.velocityTimeConstant)),
      m_zeroVelocityTime(seconds(settings.zeroVelocityTime)),
      m_gyroBias(settings.initialGyroBias) {
}

void ImuProcessor::reset() {
    m_initialized = false;
    m_stillFor = 0;
    m_hasPose = false;
    m_position = Vector3::zero();
}

ImuState ImuProcessor::process(const ImuSample& sample) {
    const Vector3 acceleration = m_mounting.toRadar(sample.acceleration);
    const Vector3 rawRate = m_mounting.toRadar(sample.angularRate);

    const float dt = seconds(sample.timestamp - m_last);
    m_last = sample.timestamp;

    if (!m_initialized || !(dt >= 0) || dt > m_maxGap) {
        initialize(acceleration, rawRate);
        return state(sample, acceleration, rawRate - m_gyroBias);
    }

    const float accelerationNorm = acceleration.length();
    updateBias(rawRate, accelerationNorm, dt);
    const Vector3 rate = rawRate - m_gyroBias;

    // A direction fixed in the room, seen from a body turning at ω, turns at −ω:
    // du/dt = −ω × u = u × ω.
    const Vector3 omega = rate * (std::numbers::pi_v<float> / 180.0f);
    m_up += MMWave::cross(m_up, omega) * dt;

    if (std::abs(accelerationNorm - 1.0f) < m_settings.accelerometerTrustBand) {
        const float k = std::min(dt / m_tiltTimeConstant, 1.0f);
        m_up += (acceleration / accelerationNorm - m_up) * k;
    }

    m_up = MMWave::normalize(m_up);

    track(acceleration, rate, omega, dt);
    return state(sample, acceleration, rate);
}

ImuState ImuProcessor::state(const ImuSample& sample, Vector3 acceleration, Vector3 rate) const {
    // The filter tracks the IMU chip; the radar sits radarOffset from it, so turning the device
    // also swings the radar around the chip: p + R·r, and v + R·(ω × r).
    const Vector3 offset = m_mounting.radarOffset;
    const Vector3 omega = rate * (std::numbers::pi_v<float> / 180.0f);
    const Vector3 position = m_position + MMWave::transform(offset, m_orientation);
    const Vector3 velocity =
        m_velocity + MMWave::transform(MMWave::cross(omega, offset), m_orientation);
    return {sample.timestamp,   acceleration,  rate,     m_up,
            sample.temperature, m_orientation, velocity, position};
}

void ImuProcessor::track(Vector3 acceleration, Vector3 rate, Vector3 omega, float dt) {
    const float turnRate = omega.length();
    if (turnRate > 0) {
        // The step is in the body's own axes, so it comes first: orientation·step.
        const Quaternion step = Quaternion::fromAxisAngle(omega / turnRate, turnRate * dt);
        m_orientation = MMWave::normalize(MMWave::concatenate(step, m_orientation));
    }

    // Gravity as this orientation has it, not "up": while the hand pushes, the accelerometer
    // tilts "up" towards the push for a while, which would take part of the push for gravity.
    const Vector3 gravity = MMWave::transform(Vector3::unitZ(), m_orientation.conjugate());
    const Vector3 linear = acceleration - gravity;
    const float linearNorm = linear.length();

    // Quiet is judged against "up", which the accelerometer always brings back, so an
    // orientation that is off cannot keep itself from ever being corrected.
    const bool quiet = (acceleration - m_up).length() < m_settings.zeroVelocityAcceleration &&
                       rate.length() < m_settings.zeroVelocityAngularRate;
    m_quietFor = quiet ? m_quietFor + dt : 0;

    if (quiet) {
        // The accelerometer reads gravity alone only now, so only now may it correct the tilt.
        const float k = std::min(dt / m_tiltTimeConstant, 1.0f);
        const Vector3 measured = MMWave::normalize(acceleration);
        m_orientation = level(m_orientation, MMWave::normalize(gravity + (measured - gravity) * k));
    }

    if (m_quietFor >= m_zeroVelocityTime) {
        m_velocity = Vector3::zero();
    } else {
        if (linearNorm > m_settings.accelerationDeadband)
            m_velocity += MMWave::transform(linear, m_orientation) * (StandardGravity * dt);
        m_velocity = m_velocity * std::exp(-dt / m_velocityTimeConstant);
    }

    m_position += m_velocity * dt;
}

void ImuProcessor::initialize(Vector3 acceleration, Vector3 rawRate) {
    const float norm = acceleration.length();
    m_up = norm > 0.5f ? acceleration / norm : Vector3::unitZ();
    m_fastMean = rawRate;
    m_fastVariance = Vector3::zero();
    m_stillFor = 0;
    m_initialized = true;

    // Heading and position carry over a gap in the data; only the tilt starts again from
    // gravity, and the velocity, unknown across the gap, from zero.
    m_orientation =
        m_hasPose ? level(m_orientation, m_up) : TiltMath::shortestArc(m_up, Vector3::unitZ());
    m_hasPose = true;
    m_velocity = Vector3::zero();
    m_quietFor = 0;
}

void ImuProcessor::updateBias(Vector3 rawRate, float accelerationNorm, float dt) {
    if (!m_settings.learnGyroBias) return;

    const float a = std::min(dt / FastWindowSeconds, 1.0f);
    m_fastMean += (rawRate - m_fastMean) * a;
    const Vector3 deviation = rawRate - m_fastMean;
    m_fastVariance += (deviation * deviation - m_fastVariance) * a;

    const float noise = m_settings.stillAngularRateNoise;
    const float worstVariance =
        std::max(m_fastVariance.x, std::max(m_fastVariance.y, m_fastVariance.z));
    const bool still = worstVariance < noise * noise &&
                       std::abs(accelerationNorm - 1.0f) < m_settings.stillAccelerationBand;

    m_stillFor = still ? m_stillFor + dt : 0;
    if (m_stillFor >= m_stillTime)
        m_gyroBias += (m_fastMean - m_gyroBias) * std::min(dt / m_biasTimeConstant, 1.0f);
}

}  // namespace Sarwave::Core::Imu::Fusion
