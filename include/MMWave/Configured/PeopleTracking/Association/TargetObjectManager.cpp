#include "MMWave/Configured/PeopleTracking/Association/TargetObjectManager.hpp"

#include <array>

#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Association {

using Pointing::PointNotAssociated;

std::vector<const TargetObject*> TargetObjectManager::allTargetObjects() const {
    std::vector<const TargetObject*> objects;
    objects.reserve(m_targetObjects.size());
    m_targetObjects.forEach(
        [&](int, const std::unique_ptr<TargetObject>& obj) { objects.push_back(obj.get()); });
    return objects;
}

void TargetObjectManager::writeLooseObjectIds(std::vector<int>& objectIds) const {
    objectIds.clear();
    m_unwiredTargetObjects.forEach(
        [&](TargetObject* obj) { objectIds.push_back(obj->objectId()); });
}

TargetObject& TargetObjectManager::getTargetObject(int objectId) {
    auto* obj = m_targetObjects.tryGet(objectId);
    if (!obj) throw KeyNotFoundException::forKey(std::to_string(objectId));
    return **obj;
}

TargetObject* TargetObjectManager::tryGetTargetObject(int objectId) {
    auto* obj = m_targetObjects.tryGet(objectId);
    return obj ? obj->get() : nullptr;
}

int TargetObjectManager::wireIdToObjectId(TlvTargetIndex wireId) const {
    switch (static_cast<PointNotAssociated>(wireId)) {
        case PointNotAssociated::PointSnrWeak:
            return static_cast<int>(NonObjectPoint::PointSnrWeak);
        case PointNotAssociated::PointOutOfBounds:
            return static_cast<int>(NonObjectPoint::PointOutOfBounds);
        case PointNotAssociated::PointLikelyNoise:
            return static_cast<int>(NonObjectPoint::PointLikelyNoise);
        default:
            break;
    }

    if (const int* id = m_wireId2ObjectId.tryGet(wireId)) return *id;
    throw KeyNotFoundException("Unrecognised wire id (" + wireId.toString() +
                               "). Declare the wire ID first.");
}

void TargetObjectManager::wireIdToObjectIdBatched(
    const Tlv::TlvContentRange<TlvTargetIndex>& wireIds, std::span<int> objectIds) const {
    int i = 0;
    for (TlvTargetIndex wireId : wireIds) {
        objectIds[static_cast<std::size_t>(i++)] = wireIdToObjectId(wireId);
    }
}

void TargetObjectManager::removeUnwiredIds(std::span<const int> droppedIds) {
    for (auto i = static_cast<std::ptrdiff_t>(droppedIds.size()) - 1; i >= 0; --i) {
        const int objId = droppedIds[static_cast<std::size_t>(i)];
        TargetObject* targetObject = &getTargetObject(objId);
        m_unwiredTargetObjects.remove(targetObject);
        m_targetObjects.remove(objId);
    }
}

void TargetObjectManager::evaluateWireIds(std::span<const TlvTargetIndex> wireIds,
                                          std::vector<TlvTargetIndex>& droppedIds,
                                          std::vector<TlvTargetIndex>& newIds) const {
    std::array<bool, 256> wireIdPresent{};
    droppedIds.clear();
    newIds.clear();

    for (TlvTargetIndex wId : wireIds) {
        wireIdPresent[wId.index] = true;

        if (m_wireId2ObjectId.contains(wId)) continue;

        newIds.push_back(wId);
    }

    m_wireId2ObjectId.forEach([&](TlvTargetIndex key, int) {
        if (!wireIdPresent[key.index]) droppedIds.push_back(key);
    });
}

void TargetObjectManager::allCurrentWireIds(std::vector<TlvTargetIndex>& droppedIds) const {
    droppedIds.clear();
    m_wireId2ObjectId.forEach([&](TlvTargetIndex key, int) { droppedIds.push_back(key); });
}

void TargetObjectManager::dropWireIds(std::span<const TlvTargetIndex> droppedIds) {
    for (TlvTargetIndex wireId : droppedIds) {
        const int* objId = m_wireId2ObjectId.tryGet(wireId);  // let it throw
        if (!objId) throw KeyNotFoundException::forKey(wireId.toString());
        TargetObject& targetObject = getTargetObject(*objId);
        targetObject.setState(TargetObject::TargetState::Dropped);
        m_unwiredTargetObjects.add(&targetObject);
        m_wireId2ObjectId.remove(wireId);
    }
}

void TargetObjectManager::declareWireIds(std::span<const TlvTargetIndex> newIds) {
    for (TlvTargetIndex newId : newIds) declareNewTargetObject(newId);
}

void TargetObjectManager::declareNewTargetObject(TlvTargetIndex wireId) {
    if (m_wireId2ObjectId.contains(wireId)) {
        throw InvalidOperationException("Wire id already exists. Risks overwriting.");
    }

    m_targetObjects.add(m_idIncrement, std::make_unique<TargetObject>(m_idIncrement));
    m_wireId2ObjectId.add(wireId, m_idIncrement);

    m_idIncrement++;
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Association
