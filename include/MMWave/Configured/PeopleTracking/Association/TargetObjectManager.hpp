#pragma once

#include <memory>
#include <span>
#include <vector>

#include "MMWave/Common/DotNetCollections.hpp"
#include "MMWave/Configured/PeopleTracking/Association/TargetObject.hpp"
#include "MMWave/Configured/PeopleTracking/Tlv/TlvTypes.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Association {

/// Maps the tracker's wire ids (reused by the device as soon as a track ends) to stable object
/// ids (handed out once, counting up), and keeps the TargetObject for each object id. A target
/// whose wire id has gone is "loose": kept, marked Dropped, until no point cloud refers to it.
class TargetObjectManager {
   public:
    using TlvTargetIndex = Tlv::TlvTargetIndex;

    [[nodiscard]] int looseObjectCount() const {
        return static_cast<int>(m_unwiredTargetObjects.size());
    }

    [[nodiscard]] int wireIdCount() const {
        return static_cast<int>(m_wireId2ObjectId.size());
    }

    /// Every target, in the same order the C# Dictionary enumerates them.
    [[nodiscard]] std::vector<const TargetObject*> allTargetObjects() const;

    /// Replaces the content of `objectIds` with the ids of the loose targets.
    void writeLooseObjectIds(std::vector<int>& objectIds) const;

    /// Throws KeyNotFoundException for an unknown id.
    TargetObject& getTargetObject(int objectId);

    /// Null for an unknown id.
    TargetObject* tryGetTargetObject(int objectId);

    /// The NonObjectPoint id for 253..255, otherwise the object id the wire id is declared
    /// under. Throws KeyNotFoundException for an undeclared wire id.
    [[nodiscard]] int wireIdToObjectId(TlvTargetIndex wireId) const;

    void wireIdToObjectIdBatched(const Tlv::TlvContentRange<TlvTargetIndex>& wireIds,
                                 std::span<int> objectIds) const;

    // ---- used by CombinedManager -----------------------------------------------------------
    void removeUnwiredIds(std::span<const int> droppedIds);

    /// Splits one frame's wire ids into those not seen before (`newIds`) and the previously
    /// declared ones that are now missing (`droppedIds`).
    void evaluateWireIds(std::span<const TlvTargetIndex> wireIds,
                         std::vector<TlvTargetIndex>& droppedIds,
                         std::vector<TlvTargetIndex>& newIds) const;

    void allCurrentWireIds(std::vector<TlvTargetIndex>& droppedIds) const;

    void dropWireIds(std::span<const TlvTargetIndex> droppedIds);

    void declareWireIds(std::span<const TlvTargetIndex> newIds);

   private:
    void declareNewTargetObject(TlvTargetIndex wireId);

    int m_idIncrement = 0;

    DotNetHashSet<TargetObject*> m_unwiredTargetObjects;
    DotNetDictionary<int, std::unique_ptr<TargetObject>> m_targetObjects;
    DotNetDictionary<TlvTargetIndex, int> m_wireId2ObjectId;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Association
