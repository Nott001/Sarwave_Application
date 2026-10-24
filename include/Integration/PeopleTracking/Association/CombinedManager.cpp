#include "Integration/PeopleTracking/Association/CombinedManager.hpp"

namespace Sarwave::Integration::PeopleTracking::Association {

using Core::Imu::ImuPose;
using Core::MMWave::Vector3;
using Tlv::TlvTargetIndex;

void CombinedManager::update(const Streaming::Frame& frame, TimeSpan timestamp) {
    update(frame, timestamp, std::nullopt);
}

void CombinedManager::update(const Streaming::Frame& frame, TimeSpan timestamp,
                             const std::optional<ImuFrameState>& imu) {
    // Unpacked even if refused, so a malformed frame is reported whether or not the device moved.
    const EntryPackage package = EntryPackage::unpack(frame);
    m_holdSteady = m_holdSteadyWarning.update(timestamp, imu);

    const Quaternion leveling = imu ? imu->tilt.rotation() : Quaternion::identity();
    const std::optional<ImuPose> pose = imu ? imu->pose : std::nullopt;

    // How the room moved in sensor axes since the last frame with a pose. A pose older than
    // that one means a replay looped, and there is nothing to go on.
    std::optional<RigidMotion> motion;
    if (pose && m_lastPose && pose->timestamp >= m_lastPose->timestamp)
        motion = pose->motionSince(*m_lastPose);

    if (imu && !imu->accepted)
        refuse(timestamp, leveling, motion);
    else
        apply(package, timestamp, leveling, motion);
    m_lastPose = pose;
    m_lastLeveling = leveling;
}

void CombinedManager::update(const EntryPackage& package, TimeSpan timestamp) {
    update(package, timestamp, Quaternion::identity());
}

void CombinedManager::update(const EntryPackage& package, TimeSpan timestamp, Quaternion leveling) {
    // Nothing says how the device moved, so a later frame's motion would not hold for this one.
    m_lastPose.reset();
    apply(package, timestamp, leveling, std::nullopt);
    m_lastLeveling = leveling;
}

void CombinedManager::apply(const EntryPackage& package, TimeSpan timestamp, Quaternion leveling,
                            const std::optional<RigidMotion>& motion) {
    if (motion) {
        // The stored clouds are in the last frame's levelled axes; the new one comes in its own.
        m_pointEntryRange.transform(
            motion->reframed(m_lastLeveling, Vector3::zero(), leveling, Vector3::zero()));
    }

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

    if (motion) {
        // Dropped targets, those just dropped included, were last placed in the last frame.
        const TrackerFrame& tracker = trackerFrame;
        m_targetObjectManager.transformUnwired(
            motion->reframed(tracker.sensorRotation, tracker.sensorOrigin, tracker.sensorRotation,
                             tracker.sensorOrigin));
    }

    update(timestamp);
}

void CombinedManager::refuse(TimeSpan timestamp, Quaternion leveling,
                             const std::optional<RigidMotion>& motion) {
    if (motion) {
        m_pointEntryRange.transform(
            motion->reframed(m_lastLeveling, Vector3::zero(), leveling, Vector3::zero()));

        // Targets still reported are moved too: nothing updates them until a frame is used.
        const TrackerFrame& tracker = trackerFrame;
        m_targetObjectManager.transformAll(
            motion->reframed(tracker.sensorRotation, tracker.sensorOrigin, tracker.sensorRotation,
                             tracker.sensorOrigin));
    }

    skipFrame(timestamp);
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

}  // namespace Sarwave::Integration::PeopleTracking::Association
