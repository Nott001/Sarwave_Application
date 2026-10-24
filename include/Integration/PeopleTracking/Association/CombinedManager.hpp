#pragma once

#include <optional>
#include <vector>

#include "Imu/Fusion/RigidMotion.hpp"
#include "Imu/Gating/HoldSteadyWarning.hpp"
#include "Imu/Gating/MotionGate.hpp"
#include "Imu/ImuPose.hpp"
#include "Integration/PeopleTracking/Association/PointEntry.hpp"
#include "Integration/PeopleTracking/Association/TargetObjectManager.hpp"
#include "Integration/PeopleTracking/Association/TrackerFrame.hpp"
#include "Integration/PeopleTracking/EntryPackage.hpp"
#include "MMWave/Common/Numerics.hpp"
#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Configured/PeopleTracking/Tlv/TlvTypes.hpp"
#include "MMWave/Streaming/Frame.hpp"

namespace Sarwave::Integration::PeopleTracking::Association {
using Core::Imu::Fusion::RigidMotion;
using Core::Imu::Gating::ImuFrameState;
using Core::MMWave::Quaternion;
using Core::MMWave::TimeSpan;

/// Follows the tracker across frames: which targets exist (with stable object ids), the point
/// clouds of the last `duration` and which target each point was assigned to, and presence.
class CombinedManager {
   public:
    explicit CombinedManager(TimeSpan duration) : duration(duration) {
    }

    [[nodiscard]] bool presence() const {
        return m_presence;
    }

    /// The operator should hold the device steady: as of the latest frame given to
    /// update(const Frame&, TimeSpan, const std::optional<ImuFrameState>&), the motion gate has
    /// been refusing frames because the device moved for long enough to be more than a knock;
    /// see HoldSteadyWarning. Frames are refused from the first sign of motion either way.
    [[nodiscard]] bool holdSteady() const {
        return m_holdSteady;
    }

    /// Where the device's tracker takes the sensor to be, from the cfg's sensorPosition; targets
    /// the device no longer reports are moved in its coordinates. Set it before frames arrive.
    TrackerFrame trackerFrame = TrackerFrame::sensorAxes();

    /// How long point clouds are kept. Public and writable, as in the C# class.
    TimeSpan duration;

    /// Every target, active and dropped, in the order the C# version lists them.
    [[nodiscard]] std::vector<const TargetObject*> allTargetObjects() const {
        return m_targetObjectManager.allTargetObjects();
    }

    /// The kept point clouds, oldest first.
    [[nodiscard]] const PointEntryRange::Entries& pointClouds() const {
        return m_pointEntryRange.entries();
    }

    void update(const Streaming::Frame& frame, TimeSpan timestamp);

    /// Applies a frame with what the IMU says about it. Its points are levelled by the device's
    /// tilt, and what is kept from earlier frames (their points, and the targets the device no
    /// longer reports) is moved by how the device turned and moved since, so it stays where it
    /// was in the room. The movement is a rough estimate; the turning is not. Without IMU data
    /// the frame is applied as it is.
    ///
    /// A frame the motion gate refused is not used: moving the device gives static surroundings
    /// an apparent Doppler velocity that looks like human micro-motion, so its points, targets
    /// and presence are left out, as the thesis excludes such frames from classification. What
    /// is already held, targets the device still reports included, is moved with the device all
    /// the same, so the last steady view stays where it was in the room until the device is
    /// steady again.
    ///
    /// Throws FrameEntryMalformedException when the frame is malformed. Nothing has been applied
    /// from it then.
    /// imu: the motion gate's verdict on this frame; nullopt without an IMU.
    void update(const Streaming::Frame& frame, TimeSpan timestamp,
                const std::optional<ImuFrameState>& imu);

    void update(const EntryPackage& package, TimeSpan timestamp);

    /// leveling: rotation applied to every point's Cartesian position as its cloud is decoded,
    /// e.g. an IMU tilt correction. Identity leaves the points in sensor axes. Targets are not
    /// touched: the device places them itself, using the cfg's sensorPosition.
    void update(const EntryPackage& package, TimeSpan timestamp, Quaternion leveling);

    /// For a frame that arrived but could not be used. Nothing is taken from it: targets keep
    /// what they last had, the last cloud stays without indexes, and time still moves on.
    void skipFrame(TimeSpan timestamp);

    /// Ages out point clouds older than `duration` and forgets loose targets nothing refers to.
    void update(TimeSpan timestamp);

   private:
    /// motion: how the room moved in sensor axes since the last frame; nullopt if unknown.
    void apply(const EntryPackage& package, TimeSpan timestamp, Quaternion leveling,
               const std::optional<RigidMotion>& motion);

    /// motion: as for apply().
    void refuse(TimeSpan timestamp, Quaternion leveling, const std::optional<RigidMotion>& motion);

    PointEntryRange m_pointEntryRange;
    TargetObjectManager m_targetObjectManager;
    std::optional<std::uint32_t> m_lastFrameNumber;
    std::optional<Core::Imu::ImuPose> m_lastPose;
    Quaternion m_lastLeveling = Quaternion::identity();
    Core::Imu::Gating::HoldSteadyWarning m_holdSteadyWarning;
    bool m_presence = false;
    bool m_holdSteady = false;
};

}  // namespace Sarwave::Integration::PeopleTracking::Association
