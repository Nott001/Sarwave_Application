#pragma once

#include <climits>

#include "Imu/Fusion/RigidMotion.hpp"
#include "MMWave/Common/Numerics.hpp"
#include "MMWave/Configured/PeopleTracking/Tlv/TlvTypes.hpp"

namespace Sarwave::Integration::PeopleTracking::Association {
using Core::Imu::Fusion::RigidMotion;
using Core::MMWave::Matrix4x4;
using Core::MMWave::Vector3;
namespace Tlv = Core::MMWave::Configured::PeopleTracking::Tlv;

/// Object ids that are not targets. Serves as an equivalent for Pointing::PointNotAssociated.
enum class NonObjectPoint : int {
    PointSnrWeak = -1,
    PointOutOfBounds = -2,
    PointLikelyNoise = -3,
    PointNotYetAssigned = INT_MIN,
};

/// True for the ids the tracker uses for rejected points (weak SNR, out of bounds, noise).
/// False for PointNotYetAssigned and for real object ids.
inline bool isNonObjectPoint(int objectId) {
    switch (static_cast<NonObjectPoint>(objectId)) {
        case NonObjectPoint::PointSnrWeak:
        case NonObjectPoint::PointOutOfBounds:
        case NonObjectPoint::PointLikelyNoise:
            return true;
        default:
            return false;
    }
}

/// One tracked target, under a stable object id that outlives the tracker's reusable wire id.
class TargetObject {
   public:
    enum class TargetState {
        Active,
        Dropped,
    };

    explicit TargetObject(int id) : m_objectId(id) {
    }

    [[nodiscard]] TargetState state() const {
        return m_state;
    }

    [[nodiscard]] int objectId() const {
        return m_objectId;
    }

    float maxZ = 0;
    float minZ = 0;

    Vector3 pos;
    Vector3 vel;
    Vector3 acc;
    Matrix4x4 ec;
    float g = 0;
    float confidenceLevel = 0;

    // ---- used by TargetObjectManager / CombinedManager ------------------------------------
    void setState(TargetState state) {
        m_state = state;
    }

    void updateListing(const Tlv::TlvTargetList& target) {
        pos = target.pos();
        vel = target.vel();
        acc = target.acc();
        ec = target.ecMatrix();
        g = target.g;
        confidenceLevel = target.confidenceLevel;
    }

    void updateHeight(const Tlv::TlvTargetHeight& height) {
        maxZ = height.maxZ;
        minZ = height.minZ;
    }

    /// Moves a target the device no longer reports, or one whose latest report was not used, as
    /// the room moved relative to the device, so it stays where it was last seen. `ec` and `g`
    /// are left as they were.
    void transform(const RigidMotion& motion) {
        const Vector3 moved = motion.apply(pos);

        // The height bounds are heights in the same coordinates, so they move with the position.
        const float rise = moved.z - pos.z;
        maxZ += rise;
        minZ += rise;

        pos = moved;
        vel = motion.applyToDirection(vel);
        acc = motion.applyToDirection(acc);
    }

   private:
    TargetState m_state = TargetState::Active;
    int m_objectId;
};

}  // namespace Sarwave::Integration::PeopleTracking::Association
