#pragma once

#include "Imu/Fusion/Fusion.hpp"
#include "Imu/ImuSample.hpp"

namespace Sarwave::Core::Imu::Fusion {

/// Tuning for ImuProcessor. The defaults suit a handheld device at 200 Hz.
struct ImuProcessorSettings {
    /// How long the accelerometer takes to pull the tilt estimate back to gravity. Shorter
    /// follows the true tilt faster but lets hand jitter into it; the gyroscope carries it in
    /// between.
    TimeSpan tiltTimeConstant = TimeSpans::fromSeconds(0.5);

    /// The accelerometer only corrects the tilt while its magnitude is within this of 1 g;
    /// outside it, the device is accelerating and the reading is not gravity.
    float accelerometerTrustBand = 0.15f;

    /// Learn the gyroscope bias whenever the device is left perfectly still.
    bool learnGyroBias = true;

    /// Stillness test: every gyroscope axis must vary by less than this (°/s, standard
    /// deviation). A device held in a hand trembles more than this, so the bias is only learned
    /// while it is set down, never from a slow turn of the hand.
    float stillAngularRateNoise = 0.25f;

    /// Stillness test: acceleration magnitude within this of 1 g.
    float stillAccelerationBand = 0.03f;

    /// How long the stillness test must hold before the bias starts following the gyroscope.
    TimeSpan stillTime = TimeSpans::fromSeconds(1);

    /// How quickly the bias follows the gyroscope while still.
    TimeSpan biasTimeConstant = TimeSpans::fromSeconds(4);

    /// A bias known from an earlier run, in radar axes, °/s. Learning starts from it.
    Vector3 initialGyroBias;

    /// A gap between samples longer than this restarts the tilt estimate from the accelerometer.
    TimeSpan maxSampleGap = TimeSpans::fromSeconds(0.1);
};

/// Turns raw samples into ImuState: rotates them into radar axes, removes the gyroscope bias,
/// and tracks which way is up.
///
/// "Up" comes from a complementary filter on the gravity direction. The gyroscope turns it from
/// sample to sample, which is accurate over short spans and unaffected by the hand's
/// acceleration; the accelerometer slowly pulls it towards the measured gravity, which removes
/// the gyroscope's drift. That is all the thesis's tilt compensation needs (pitch and roll);
/// heading would need a magnetometer and is not tracked.
///
/// Live and replayed data go through the same code, so a recording processed later gives the
/// same states the live run did. Samples must arrive in time order. Not thread-safe.
class ImuProcessor {
   public:
    explicit ImuProcessor(ImuMounting mounting, ImuProcessorSettings settings = {});

    [[nodiscard]] const ImuMounting& mounting() const {
        return m_mounting;
    }

    /// The current gyroscope bias estimate, radar axes, °/s. Save it to start the next run with.
    [[nodiscard]] Vector3 gyroBias() const {
        return m_gyroBias;
    }

    /// True while the device has passed the stillness test long enough to be learning the bias.
    [[nodiscard]] bool isStill() const {
        return m_stillFor >= m_stillTime;
    }

    /// Forgets the tilt and stillness state; the next sample starts afresh. The bias is kept.
    void reset();

    ImuState process(const ImuSample& sample);

   private:
    void initialize(Vector3 acceleration, Vector3 rawRate);
    void updateBias(Vector3 rawRate, float accelerationNorm, float dt);

    ImuProcessorSettings m_settings;
    ImuMounting m_mounting;
    float m_tiltTimeConstant;
    float m_biasTimeConstant;
    float m_stillTime;
    float m_maxGap;

    Vector3 m_gyroBias;
    bool m_initialized = false;
    TimeSpan m_last{};
    Vector3 m_up;
    Vector3 m_fastMean;
    Vector3 m_fastVariance;
    float m_stillFor = 0;
};

}  // namespace Sarwave::Core::Imu::Fusion
