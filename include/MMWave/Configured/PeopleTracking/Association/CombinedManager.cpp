#include "MMWave/Configured/PeopleTracking/Association/CombinedManager.hpp"

#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Tlv/TlvRange.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Association {

using Tlv::TlvTargetIndex;
using Tlv::TlvType;

EntryPackage EntryPackage::unpack(const Streaming::Frame& frame) {
    EntryPackage package;
    // Kept raw and decoded after the loop, as the C# version does: only the last Point Cloud TLV
    // is ever decoded (and can fail).
    std::optional<MMWave::Tlv::TlvEntry> pointCloud;

    for (const MMWave::Tlv::TlvEntry& entry : MMWave::Tlv::TlvRange(frame)) {
        switch (static_cast<TlvType>(entry.type)) {
            case TlvType::TlvTargetList:
                package.targets.emplace(entry);
                break;
            case TlvType::TlvTargetIndex:
                package.indexes.emplace(entry);
                break;
            case TlvType::TlvTargetHeight:
                package.heights.emplace(entry);
                break;
            case TlvType::TlvPointCloud:
                pointCloud = entry;
                break;
            case TlvType::TlvPresenceIndication:
                package.presence = Tlv::TlvPresenceIndication::presence(entry);
                break;
            default:
                throw ArgumentOutOfRangeException(
                    "frame", "Unknown TlvType " + std::to_string(entry.type) + ".");
        }
    }

    if (pointCloud) package.pointCloud = Tlv::TlvPointCloud::decompile(*pointCloud);
    return package;
}

void CombinedManager::update(const Streaming::Frame& frame, TimeSpan timestamp) {
    const EntryPackage package = EntryPackage::unpack(frame);
    update(package, timestamp);
}

void CombinedManager::update(const EntryPackage& package, TimeSpan timestamp) {
    std::vector<TlvTargetIndex> droppedIds;

    if (package.targets) {
        const auto& targetRange = *package.targets;
        std::vector<TlvTargetIndex> wireIds;
        wireIds.reserve(static_cast<std::size_t>(targetRange.count()));
        for (const Tlv::TlvTargetList& target : targetRange) wireIds.push_back(target.targetId);

        std::vector<TlvTargetIndex> newIds;
        m_targetObjectManager.evaluateWireIds(wireIds, droppedIds, newIds);
        m_targetObjectManager.declareWireIds(newIds);

        for (const Tlv::TlvTargetList& target : targetRange) {
            const int objectId = m_targetObjectManager.wireIdToObjectId(target.targetId);
            TargetObject& obj = m_targetObjectManager.getTargetObject(objectId);
            obj.updateListing(target);
        }
    } else {
        m_targetObjectManager.allCurrentWireIds(droppedIds);
    }

    if (package.heights) {
        for (const Tlv::TlvTargetHeight& height : *package.heights) {
            const int objectId = m_targetObjectManager.wireIdToObjectId(height.targetId);
            TargetObject& obj = m_targetObjectManager.getTargetObject(objectId);
            obj.updateHeight(height);
        }
    }

    if (package.indexes) {
        const auto& indexRange = *package.indexes;
        std::vector<int> objectIds(static_cast<std::size_t>(indexRange.count()));
        m_targetObjectManager.wireIdToObjectIdBatched(indexRange, objectIds);
        if (PointEntry* last = m_pointEntryRange.last()) last->associatePoints(objectIds);
    }

    if (package.pointCloud) {
        m_pointEntryRange.add(*package.pointCloud, timestamp);
    } else {
        m_pointEntryRange.setLastToNull();
    }

    if (package.presence) m_presence = *package.presence;

    m_targetObjectManager.dropWireIds(droppedIds);
    update(timestamp);
}

void CombinedManager::update(TimeSpan timestamp) {
    m_pointEntryRange.dropOldEntries(timestamp, duration);

    std::vector<int> looseObjIds;
    m_targetObjectManager.writeLooseObjectIds(looseObjIds);

    std::vector<int> objIdsToRemove;
    m_pointEntryRange.unusedTargetObjects(looseObjIds, objIdsToRemove);
    m_targetObjectManager.removeUnwiredIds(objIdsToRemove);
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Association
