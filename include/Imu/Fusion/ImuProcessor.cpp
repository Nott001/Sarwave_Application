#include "Imu/Fusion/ImuProcessor.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace Sarwave::Core::Imu::Fusion {
namespace {

constexpr float FastWindowSeconds = 0.25f;

float seconds(TimeSpan span) {
    return static_cast<float>(static_cast<double>(span.count()) / TimeSpans::TicksPerSecond);
}

}  // namespace

ImuProcessor::ImuProcessor(ImuMounting mounting, ImuProcessorSettings settings)
    : m_settings(settings),
      m_mounting(mounting),
      m_tiltTimeConstant(seconds(settings.tiltTimeConstant)),
      m_biasTimeConstant(seconds(settings.biasTimeConstant)),
      m_stillTime(seconds(settings.stillTime)),
      m_maxGap(seconds(settings.maxSampleGap)),
      m_gyroBias(settings.initialGyroBias) {
}

void ImuProcessor::reset() {
    m_initialized = false;
    m_stillFor = 0;
}

ImuState ImuProcessor::process(const ImuSample& sample) {
    const Vector3 acceleration = m_mounting.toRadar(sample.acceleration);
    const Vector3 rawRate = m_mounting.toRadar(sample.angularRate);

    const float dt = seconds(sample.timestamp - m_last);
    m_last = sample.timestamp;

    if (!m_initialized || !(dt >= 0) || dt > m_maxGap) {
        initialize(acceleration, rawRate);
        return {sample.timestamp, acceleration, rawRate - m_gyroBias, m_up, sample.temperature};
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
    return {sample.timestamp, acceleration, rate, m_up, sample.temperature};
}

void ImuProcessor::initialize(Vector3 acceleration, Vector3 rawRate) {
    const float norm = acceleration.length();
    m_up = norm > 0.5f ? acceleration / norm : Vector3::unitZ();
    m_fastMean = rawRate;
    m_fastVariance = Vector3::zero();
    m_stillFor = 0;
    m_initialized = true;
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
