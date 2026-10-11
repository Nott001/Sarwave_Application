#pragma once

#include <optional>
#include <vector>

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

    /// Throws ArgumentOutOfRangeException for a TLV type the People Tracking demo does not
    /// send ("Unknown TlvType N."), or a TLV too short for its fixed part.
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
    void update(const EntryPackage& package, TimeSpan timestamp);

    /// Ages out point clouds older than `duration` and forgets loose targets nothing refers to.
    void update(TimeSpan timestamp);

   private:
    PointEntryRange m_pointEntryRange;
    TargetObjectManager m_targetObjectManager;
    bool m_presence = false;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Association
