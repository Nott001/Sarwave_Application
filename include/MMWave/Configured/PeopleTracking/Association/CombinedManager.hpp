#pragma once

#include <optional>
#include <vector>

#include "MMWave/Common/Numerics.hpp"
#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Configured/PeopleTracking/Association/PointEntry.hpp"
#include "MMWave/Configured/PeopleTracking/Association/TargetObjectManager.hpp"
#include "MMWave/Configured/PeopleTracking/Tlv/TlvTypes.hpp"
#include "MMWave/Streaming/Frame.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Association {

/// The People Tracking TLVs of one frame, each present or not. Views into the frame: only valid
/// while it is.
struct EntryPackage {
    std::optional<Tlv::TlvPointCloud> pointCloud;
    std::optional<Tlv::TlvContentRange<Tlv::TlvTargetList>> targets;
    std::optional<Tlv::TlvContentRange<Tlv::TlvTargetHeight>> heights;
    std::optional<Tlv::TlvContentRange<Tlv::TlvTargetIndex>> indexes;
    std::optional<bool> presence;
    std::uint32_t frameNumber = 0;

    /// Sorts the frame's TLVs into a package, checking on the way that the frame is one the
    /// device can have sent. Every rule here held for all 2,360 intact frames in the recordings
    /// described in the C# solution's docs/recorded_data.txt.
    ///
    /// Throws Exceptions::FrameEntryMalformedException when the frame breaks one of those rules.
    static EntryPackage unpack(const Streaming::Frame& frame);
};

/// Follows the tracker across frames: which targets exist (with stable object ids), the point
/// clouds of the last `duration` and which target each point was assigned to, and presence.
class CombinedManager {
   public:
    explicit CombinedManager(TimeSpan duration) : duration(duration) {
    }

    [[nodiscard]] bool presence() const {
        return m_presence;
    }

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

    /// leveling: rotation applied to every point's Cartesian position as its cloud is decoded,
    /// e.g. an IMU tilt correction. Identity leaves the points in sensor axes. Targets are not
    /// touched: the device places them itself, using the cfg's sensorPosition.
    void update(const Streaming::Frame& frame, TimeSpan timestamp, Quaternion leveling);

    void update(const EntryPackage& package, TimeSpan timestamp);

    /// leveling: see update(const Frame&, TimeSpan, Quaternion).
    void update(const EntryPackage& package, TimeSpan timestamp, Quaternion leveling);

    /// For a frame that arrived but could not be used. Nothing is taken from it: targets keep
    /// what they last had, the last cloud stays without indexes, and time still moves on.
    void skipFrame(TimeSpan timestamp);

    /// Ages out point clouds older than `duration` and forgets loose targets nothing refers to.
    void update(TimeSpan timestamp);

   private:
    PointEntryRange m_pointEntryRange;
    TargetObjectManager m_targetObjectManager;
    std::optional<std::uint32_t> m_lastFrameNumber;
    bool m_presence = false;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Association
