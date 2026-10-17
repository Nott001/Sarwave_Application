#pragma once

#include <cstdint>
#include <string>

#include "Imu/ImuSample.hpp"

/// Sarwave.Core.Imu.Fusion: from raw samples to "which way is up" and the rotation that levels
/// the radar's points. TiltMath, ImuMounting and TiltCorrection of the C# library.
namespace Sarwave::Core::Imu::Fusion {

namespace TiltMath {

/// The smallest rotation taking unit vector `from` onto unit vector `to`. Its axis is
/// perpendicular to both, so when `to` is vertical it tilts without turning: the boresight keeps
/// its compass heading.
Quaternion shortestArc(Vector3 from, Vector3 to);

}  // namespace TiltMath

/// A signed IMU axis, as printed on the chip or breakout board.
enum class ImuAxis : std::uint8_t {
    PositiveX,
    NegativeX,
    PositiveY,
    NegativeY,
    PositiveZ,
    NegativeZ,
};

/// Enum.ToString().
std::string toString(ImuAxis axis);

/// How the IMU sits relative to the radar: the rotation that turns a vector in IMU axes into the
/// same vector in radar axes (X right, Y along the boresight, Z up, as in PointValue's
/// Cartesian position).
///
/// Set it once from how the board is mounted with fromAxes(), then optionally trim the remaining
/// few degrees of misalignment with withLevelCalibration().
struct ImuMounting {
    Quaternion rotation = Quaternion::identity();

    /// The IMU's X, Y and Z already point right, along the boresight, and up.
    static ImuMounting identity() {
        return {};
    }

    /// Names the IMU axis that points the same way as each radar axis. For an IMU lying flat
    /// with its Y arrow pointing at the scene and its X arrow to the right, that is (+X, +Y, +Z);
    /// turned so its X arrow points at the scene and its Y arrow to the left, it is (−Y, +X, +Z).
    /// Throws ArgumentException when the three are not a right-handed set of distinct axes.
    static ImuMounting fromAxes(ImuAxis radarRight, ImuAxis radarForward, ImuAxis radarUp);

    /// Adds the small rotation that makes `measuredUp` read as straight up. Hold the device with
    /// the radar boresight level and not rolled, read the mean ImuState::up from a still stretch
    /// (e.g. ImuTimeline::summarize), and pass it here; the IMU's tilt relative to the radar
    /// board is then taken out of every later reading.
    [[nodiscard]] ImuMounting withLevelCalibration(Vector3 measuredUp) const;

    /// Turns a vector in IMU axes into radar axes.
    [[nodiscard]] Vector3 toRadar(Vector3 imuVector) const {
        return MMWave::transform(imuVector, rotation);
    }
};

/// Undoes how the operator is tilting the device: rotates points from radar axes into a level
/// frame whose Z is straight up and whose Y is the boresight's horizontal direction. Heights then
/// stay referenced to the floor, and a point straight ahead stays straight ahead.
///
/// Only pitch and roll are known (gravity says nothing about heading), and only they are removed.
class TiltCorrection {
   public:
    /// No IMU data: points are left as they are.
    static TiltCorrection none() {
        return {};
    }

    /// up: direction away from the floor in radar axes, e.g. a mean of ImuState::up.
    static TiltCorrection fromUp(Vector3 up);

    /// Radar axes to level axes. Identity when isValid() is false.
    [[nodiscard]] Quaternion rotation() const {
        return m_rotation;
    }

    /// Boresight elevation above the horizon: positive when the radar points upwards.
    [[nodiscard]] float pitchDegrees() const {
        return m_pitchDegrees;
    }

    /// Rotation about the boresight: positive when the radar's right side is lower.
    [[nodiscard]] float rollDegrees() const {
        return m_rollDegrees;
    }

    /// False when there was no IMU data to base it on.
    [[nodiscard]] bool isValid() const {
        return m_isValid;
    }

    /// Levels one point given in radar axes.
    [[nodiscard]] Vector3 apply(Vector3 point) const {
        return m_isValid ? MMWave::transform(point, m_rotation) : point;
    }

    /// "pitch +3.0°, roll -1.2°", or "no tilt data".
    [[nodiscard]] std::string toString() const;

   private:
    Quaternion m_rotation = Quaternion::identity();
    float m_pitchDegrees = 0;
    float m_rollDegrees = 0;
    bool m_isValid = false;
};

}  // namespace Sarwave::Core::Imu::Fusion
