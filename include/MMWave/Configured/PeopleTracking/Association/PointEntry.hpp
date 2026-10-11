#pragma once

#include <deque>
#include <memory>
#include <optional>
#include <span>
#include <unordered_set>
#include <utility>
#include <vector>

#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Configured/PeopleTracking/Association/TargetObject.hpp"
#include "MMWave/Configured/PeopleTracking/Pointing/Pointing.hpp"
#include "MMWave/Configured/PeopleTracking/Tlv/TlvTypes.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Association {

/// The points of one frame's Point Cloud TLV. Once the next frame's Target Index TLV arrives
/// they are "associated": sorted by owning object id, so each owner's points are one slice.
class PointEntry {
   public:
    PointEntry(const Tlv::TlvPointCloud& pointCloud, TimeSpan timestamp);
    PointEntry(std::vector<Pointing::PointFigure> pointFigures, TimeSpan timestamp);

    [[nodiscard]] TimeSpan timestamp() const {
        return m_timestamp;
    }

    [[nodiscard]] bool isAssociated() const {
        return m_relatedObject.has_value();
    }

    [[nodiscard]] std::span<const Pointing::PointFigure> pointValues() const {
        return m_pointFigures;
    }

    /// The points that belong to `objectId` (empty if none). Throws InvalidOperationException
    /// while the entry is not associated yet.
    [[nodiscard]] std::span<const Pointing::PointFigure> getPointsOf(int objectId) const;

    [[nodiscard]] std::span<const Pointing::PointFigure> getPointsOf(NonObjectPoint id) const {
        return getPointsOf(static_cast<int>(id));
    }

    /// The owners of this entry's points, ascending (so rejected ids, which are negative,
    /// come first). Empty while not associated. `excludeConstants` leaves out the
    /// NonObjectPoint ids.
    [[nodiscard]] std::vector<int> objectIdsUsed(bool excludeConstants) const;

    /// Takes one owner per point (same length as pointValues()). Sorts both by owner.
    /// Throws InvalidOperationException if already associated, ArgumentException on a length
    /// mismatch. Note: reorders `associatedObjectIds` as well, as the C# version does.
    void associatePoints(std::span<int> associatedObjectIds);

   private:
    struct ArraySector {
        int index;
        int length;
    };

    // Owner id -> its slice, in ascending id order (the order .NET's Dictionary enumerates
    // them in, since they are inserted sorted).
    std::optional<std::vector<std::pair<int, ArraySector>>> m_relatedObject;
    std::vector<Pointing::PointFigure> m_pointFigures;
    TimeSpan m_timestamp;
};

/// The point clouds of the last `duration`, oldest first.
class PointEntryRange {
   public:
    using Entries = std::deque<std::shared_ptr<PointEntry>>;

    [[nodiscard]] const Entries& entries() const {
        return m_pointsEntries;
    }

    [[nodiscard]] int count() const {
        return static_cast<int>(m_pointsEntries.size());
    }

    /// The last point cloud data of the frame. Null if the previous frame did not send any at all.
    [[nodiscard]] PointEntry* last() const {
        return m_last.get();
    }

    /// Writes the ids from `objectIds` that no remaining entry has points for into `unused`.
    void unusedTargetObjects(std::span<const int> objectIds, std::vector<int>& unused);

    void setLastToNull() {
        m_last.reset();
    }

    void add(const Tlv::TlvPointCloud& pointCloud, TimeSpan timestamp);

    void dropOldEntries(TimeSpan timestamp, TimeSpan duration);

   private:
    std::unordered_set<int> m_usedScratch;  // reused across calls, not reallocated each time
    Entries m_pointsEntries;
    std::shared_ptr<PointEntry> m_last;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Association
