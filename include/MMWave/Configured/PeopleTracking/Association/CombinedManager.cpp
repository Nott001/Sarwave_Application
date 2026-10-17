#include "MMWave/Configured/PeopleTracking/Association/CombinedManager.hpp"

#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"
#include "MMWave/Tlv/TlvRange.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Association {

using Tlv::TlvTargetIndex;
using Tlv::TlvType;

namespace {

using Exceptions::FrameEntryMalformedException;
using Reason = FrameEntryMalformedException::Reason;

/// Track IDs run from 0 to this; the values above it are markers or invalid.
constexpr std::uint32_t MaxTrackId = 249;

std::uint32_t readLe32(std::span<const std::uint8_t> b) {
    return static_cast<std::uint32_t>(b[0]) | static_cast<std::uint32_t>(b[1]) << 8 |
           static_cast<std::uint32_t>(b[2]) << 16 | static_cast<std::uint32_t>(b[3]) << 24;
}

std::string typeName(const MMWave::Tlv::TlvEntry& entry) {
    return Tlv::toString(static_cast<TlvType>(entry.type));
}

void requireFirst(bool alreadySeen, const MMWave::Tlv::TlvEntry& entry, std::uint32_t frameNumber) {
    if (alreadySeen) {
        throw FrameEntryMalformedException(
            Reason::DuplicateTlv, typeName(entry) + " appears more than once.", frameNumber);
    }
}

/// The payload must be `prefixSize` bytes plus whole records.
void requireRecords(const MMWave::Tlv::TlvEntry& entry, std::size_t recordSize,
                    std::size_t prefixSize, std::uint32_t frameNumber) {
    const std::size_t length = entry.bytes.size();
    if (length < prefixSize || (length - prefixSize) % recordSize != 0) {
        throw FrameEntryMalformedException(Reason::PayloadLength,
                                           typeName(entry) + " is " + std::to_string(length) +
                                               " bytes, which is not " +
                                               std::to_string(prefixSize) + " plus a multiple of " +
                                               std::to_string(recordSize) + ".",
                                           frameNumber);
    }
}

/// The ID opens each record as four bytes on the wire. All four are read here, although the
/// record types keep only the first, so that garbage in the other three is not missed.
void requireTrackIds(const MMWave::Tlv::TlvEntry& entry, std::size_t recordSize,
                     std::uint32_t frameNumber) {
    const auto bytes = entry.bytes;
    for (std::size_t offset = 0; offset < bytes.size(); offset += recordSize) {
        const std::uint32_t id = readLe32(bytes.subspan(offset));
        if (id > MaxTrackId) {
            throw FrameEntryMalformedException(Reason::TrackId,
                                               typeName(entry) + " record " +
                                                   std::to_string(offset / recordSize) +
                                                   " has track ID " + std::to_string(id) + ".",
                                               frameNumber);
        }
    }
}

void requirePointIndexes(const MMWave::Tlv::TlvEntry& entry, std::uint32_t frameNumber) {
    const auto bytes = entry.bytes;
    for (std::size_t i = 0; i < bytes.size(); i++) {
        if (bytes[i] > MaxTrackId &&
            bytes[i] < static_cast<std::uint8_t>(Pointing::PointNotAssociated::PointSnrWeak)) {
            throw FrameEntryMalformedException(
                Reason::PointIndex,
                "Target index " + std::to_string(i) + " is " + std::to_string(bytes[i]) + ".",
                frameNumber);
        }
    }
}

bool readPresence(const MMWave::Tlv::TlvEntry& entry, std::uint32_t frameNumber) {
    if (entry.bytes.size() != sizeof(std::uint32_t)) {
        throw FrameEntryMalformedException(
            Reason::PayloadLength,
            typeName(entry) + " is " + std::to_string(entry.bytes.size()) + " bytes instead of 4.",
            frameNumber);
    }

    const std::uint32_t value = readLe32(entry.bytes);
    if (value > 1) {
        throw FrameEntryMalformedException(
            Reason::Presence, "Presence is " + std::to_string(value) + ".", frameNumber);
    }
    return value != 0;
}

/// The device sends the heights with the target list, for the same targets in the same order.
void requireMatchingHeights(const EntryPackage& package, std::uint32_t frameNumber) {
    const int targetCount = package.targets ? package.targets->count() : 0;
    const int heightCount = package.heights ? package.heights->count() : 0;

    if (package.targets.has_value() != package.heights.has_value() || targetCount != heightCount) {
        throw FrameEntryMalformedException(Reason::HeightMismatch,
                                           std::to_string(targetCount) +
                                               " targets are listed but " +
                                               std::to_string(heightCount) + " heights.",
                                           frameNumber);
    }

    for (int i = 0; i < targetCount; i++) {
        const TlvTargetIndex listed = (*package.targets)[i].targetId;
        const TlvTargetIndex measured = (*package.heights)[i].targetId;
        if (listed != measured) {
            throw FrameEntryMalformedException(Reason::HeightMismatch,
                                               "Target " + std::to_string(i) + " is track " +
                                                   listed.toString() + " in the list but track " +
                                                   measured.toString() + " in the heights.",
                                               frameNumber);
        }
    }
}

}  // namespace

EntryPackage EntryPackage::unpack(const Streaming::Frame& frame) {
    const Streaming::FrameHeader header = frame.header();
    const std::uint32_t frameNumber = header.frameNumber;

    EntryPackage package;
    package.frameNumber = frameNumber;
    std::uint32_t entryCount = 0;

    for (const MMWave::Tlv::TlvEntry& entry : MMWave::Tlv::TlvRange(frame)) {
        entryCount++;

        switch (static_cast<TlvType>(entry.type)) {
            case TlvType::TlvTargetList:
                requireFirst(package.targets.has_value(), entry, frameNumber);
                requireRecords(entry, sizeof(Tlv::TlvTargetList), 0, frameNumber);
                requireTrackIds(entry, sizeof(Tlv::TlvTargetList), frameNumber);
                package.targets.emplace(entry);
                break;
            case TlvType::TlvTargetIndex:
                requireFirst(package.indexes.has_value(), entry, frameNumber);
                requirePointIndexes(entry, frameNumber);
                package.indexes.emplace(entry);
                break;
            case TlvType::TlvTargetHeight:
                requireFirst(package.heights.has_value(), entry, frameNumber);
                requireRecords(entry, sizeof(Tlv::TlvTargetHeight), 0, frameNumber);
                requireTrackIds(entry, sizeof(Tlv::TlvTargetHeight), frameNumber);
                package.heights.emplace(entry);
                break;
            case TlvType::TlvPointCloud:
                requireFirst(package.pointCloud.has_value(), entry, frameNumber);
                requireRecords(entry, sizeof(Pointing::CompressedPoint),
                               sizeof(Pointing::PointUnit), frameNumber);
                package.pointCloud = Tlv::TlvPointCloud::decompile(entry);
                break;
            case TlvType::TlvPresenceIndication:
                requireFirst(package.presence.has_value(), entry, frameNumber);
                package.presence = readPresence(entry, frameNumber);
                break;
            default:
                throw FrameEntryMalformedException(
                    Reason::UnknownTlv,
                    "TLV type " + std::to_string(entry.type) +
                        " is not part of the people tracking output.",
                    frameNumber);
        }
    }

    // TlvRange ends quietly at the first TLV that does not fit in the packet, which would
    // otherwise pass for a frame that simply has fewer TLVs.
    if (entryCount != header.numTlvs) {
        throw FrameEntryMalformedException(Reason::Truncated,
                                           "The header counts " + std::to_string(header.numTlvs) +
                                               " TLVs but only " + std::to_string(entryCount) +
                                               " fit in the packet.",
                                           frameNumber);
    }

    const int pointCount = package.pointCloud ? package.pointCloud->points.count() : 0;
    if (static_cast<std::uint32_t>(pointCount) != header.numDetectedObj) {
        throw FrameEntryMalformedException(
            Reason::PointCount,
            "The header gives " + std::to_string(header.numDetectedObj) +
                " points but the point cloud holds " + std::to_string(pointCount) + ".",
            frameNumber);
    }

    requireMatchingHeights(package, frameNumber);
    return package;
}

void CombinedManager::update(const Streaming::Frame& frame, TimeSpan timestamp) {
    update(frame, timestamp, Quaternion::identity());
}

void CombinedManager::update(const Streaming::Frame& frame, TimeSpan timestamp,
                             Quaternion leveling) {
    const EntryPackage package = EntryPackage::unpack(frame);
    update(package, timestamp, leveling);
}

void CombinedManager::update(const EntryPackage& package, TimeSpan timestamp) {
    update(package, timestamp, Quaternion::identity());
}

void CombinedManager::update(const EntryPackage& package, TimeSpan timestamp, Quaternion leveling) {
    // The index TLV describes the points of the frame sent just before this one. If that frame
    // never arrived, the last cloud held here is an older one and must not be given these
    // indexes. Also true when the sensor restarts or a recording loops.
    if (m_lastFrameNumber && package.frameNumber != *m_lastFrameNumber + 1) {
        m_pointEntryRange.setLastToNull();
    }
    m_lastFrameNumber = package.frameNumber;

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

    // Without a cloud to receive them the indexes are not looked up at all: after a missed
    // frame they can name a track this manager was never told about.
    if (PointEntry* lastCloud = m_pointEntryRange.last(); package.indexes && lastCloud) {
        const auto& indexRange = *package.indexes;
        std::vector<int> objectIds(static_cast<std::size_t>(indexRange.count()));
        m_targetObjectManager.wireIdToObjectIdBatched(indexRange, objectIds);
        lastCloud->associatePoints(objectIds);
    }

    if (package.pointCloud) {
        m_pointEntryRange.add(*package.pointCloud, timestamp, leveling);
    } else {
        m_pointEntryRange.setLastToNull();
    }

    if (package.presence) m_presence = *package.presence;

    m_targetObjectManager.dropWireIds(droppedIds);
    update(timestamp);
}

void CombinedManager::skipFrame(TimeSpan timestamp) {
    m_pointEntryRange.setLastToNull();
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
