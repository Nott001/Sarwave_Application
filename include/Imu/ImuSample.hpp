#pragma once

#include "MMWave/Common/Numerics.hpp"
#include "MMWave/Common/TimeSpan.hpp"

/// Sarwave.Core.Imu: the TDK InvenSense ICM-42688-P six-axis IMU fixed to the IWR6843AOP --
/// motion gating, tilt compensation and IMU recording. It shares only the header-only numeric
/// and time types with the radar code, so it can be used on its own.
namespace Sarwave::Core::Imu {

using MMWave::Quaternion;
using MMWave::TimeSpan;
using MMWave::Vector3;
namespace TimeSpans = MMWave::TimeSpans;

/// One raw IMU sample on the host clock, in the IMU's own axes. This is what gets recorded, so a
/// recording can be processed again later with a different mounting or different settings.
struct ImuSample {
    /// When the chip took the sample, on the same clock as the radar frames.
    TimeSpan timestamp{};
    /// Specific force in g, IMU axes.
    Vector3 acceleration;
    /// °/s, IMU axes, gyroscope bias not removed.
    Vector3 angularRate;
    /// Die temperature, °C.
    float temperature = 0;

    friend bool operator==(const ImuSample&, const ImuSample&) = default;
};

/// One processed sample: rotated into the radar's axes (X right, Y along the boresight, Z up),
/// gyroscope bias removed, and with the filtered direction of "up" at that instant.
struct ImuState {
    TimeSpan timestamp{};
    /// Specific force in g, radar axes. Equals `up` when the device is still.
    Vector3 acceleration;
    /// °/s, radar axes, bias removed.
    Vector3 angularRate;
    /// Unit vector pointing away from the floor, in radar axes.
    Vector3 up;
    float temperature = 0;

    /// Acceleration with gravity taken out, in g: what moving the device adds.
    [[nodiscard]] Vector3 linearAcceleration() const {
        return acceleration - up;
    }
};

}  // namespace Sarwave::Core::Imu
