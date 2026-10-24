#pragma once

#include "MMWave/Common/Numerics.hpp"

namespace Sarwave::Core::Imu::Fusion {

/// A rotation followed by a translation: p' = rotation·p + translation.
struct RigidMotion {
    MMWave::Quaternion rotation = MMWave::Quaternion::identity();
    MMWave::Vector3 translation;

    static RigidMotion identity() {
        return {};
    }

    [[nodiscard]] MMWave::Vector3 apply(MMWave::Vector3 point) const {
        return MMWave::transform(point, rotation) + translation;
    }

    /// Rotates a direction, such as a velocity; the translation does not apply to it.
    [[nodiscard]] MMWave::Vector3 applyToDirection(MMWave::Vector3 direction) const {
        return MMWave::transform(direction, rotation);
    }

    /// The same motion for coordinates kept in another frame, where a point p in the original
    /// axes is stored as frame·p + origin. `before` is the frame the coordinates are in now and
    /// `after` the one they should end up in: the two differ when, say, each radar frame's points
    /// are levelled by that frame's own tilt.
    [[nodiscard]] RigidMotion reframed(MMWave::Quaternion before, MMWave::Vector3 originBefore,
                                       MMWave::Quaternion after,
                                       MMWave::Vector3 originAfter) const {
        // after·(rotation·before⁻¹·(p − originBefore) + translation) + originAfter
        const MMWave::Quaternion r = MMWave::normalize(
            MMWave::concatenate(MMWave::concatenate(before.conjugate(), rotation), after));
        const MMWave::Vector3 t = MMWave::transform(translation, after) + originAfter -
                                  MMWave::transform(originBefore, r);
        return {r, t};
    }

    friend constexpr bool operator==(const RigidMotion&, const RigidMotion&) = default;
};

}  // namespace Sarwave::Core::Imu::Fusion
