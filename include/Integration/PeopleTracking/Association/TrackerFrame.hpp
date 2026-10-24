#pragma once

#include <numbers>

#include "MMWave/Common/Numerics.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/SensorPosition.hpp"

namespace Sarwave::Integration::PeopleTracking::Association {

/// Where the device's tracker takes the sensor to be. Targets come in the tracker's coordinates,
/// in which a point p in sensor axes is sensorRotation·p + sensorOrigin; point clouds stay in
/// sensor axes.
struct TrackerFrame {
    /// Turns sensor axes into tracker axes.
    Core::MMWave::Quaternion sensorRotation = Core::MMWave::Quaternion::identity();
    /// The sensor's position in tracker coordinates, metres.
    Core::MMWave::Vector3 sensorOrigin;

    /// Tracker and sensor axes the same, as if the cfg had no sensorPosition.
    static TrackerFrame sensorAxes() {
        return {};
    }

    /// From the cfg's sensorPosition: the sensor sensorHeight above the tracker's origin on the
    /// floor, tilted down by elevTilt, then turned by azimTilt about the vertical. The order of
    /// the two tilts is assumed; every cfg here has an azimTilt of 0.
    static TrackerFrame from(
        const Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters::SensorPosition&
            position) {
        using Core::MMWave::Quaternion;
        using Core::MMWave::Vector3;
        const Quaternion down =
            Quaternion::fromAxisAngle(Vector3::unitX(), -degreesToRadians(position.elevTilt));
        const Quaternion turn =
            Quaternion::fromAxisAngle(Vector3::unitZ(), degreesToRadians(position.azimTilt));
        return {Core::MMWave::concatenate(down, turn), {0, 0, position.sensorHeight}};
    }

    friend constexpr bool operator==(const TrackerFrame&, const TrackerFrame&) = default;

   private:
    /// float.DegreesToRadians: (degrees * π) / 180, in that order.
    static float degreesToRadians(float degrees) {
        return (degrees * std::numbers::pi_v<float>) / 180.0f;
    }
};

}  // namespace Sarwave::Integration::PeopleTracking::Association
