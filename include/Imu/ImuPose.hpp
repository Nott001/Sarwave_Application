#pragma once

#include "Imu/Fusion/RigidMotion.hpp"
#include "MMWave/Common/Numerics.hpp"
#include "MMWave/Common/TimeSpan.hpp"

namespace Sarwave::Core::Imu {

/// Where the device was and which way it faced at one instant, in room axes: Z up, with the
/// heading and the origin wherever the IMU started. Heading comes from the gyroscope alone and
/// position from integrating acceleration, so only the change between two poses close in time
/// means anything; see motionSince().
struct ImuPose {
    /// When, on the radar frames' clock.
    MMWave::TimeSpan timestamp{};
    /// Turns a vector in radar axes into room axes.
    MMWave::Quaternion orientation = MMWave::Quaternion::identity();
    /// The device's position in room axes, metres. A rough estimate.
    MMWave::Vector3 position;

    /// How anything fixed in the room moved in radar axes between `earlier` and this pose:
    /// applied to a point's radar coordinates at `earlier`, it gives the point's radar
    /// coordinates now.
    [[nodiscard]] Fusion::RigidMotion motionSince(const ImuPose& earlier) const {
        // room = earlier.orientation·p + earlier.position = orientation·p' + position.
        const MMWave::Quaternion toRadar = orientation.conjugate();
        return {MMWave::normalize(MMWave::concatenate(earlier.orientation, toRadar)),
                MMWave::transform(earlier.position - position, toRadar)};
    }

    friend constexpr bool operator==(const ImuPose&, const ImuPose&) = default;
};

}  // namespace Sarwave::Core::Imu
