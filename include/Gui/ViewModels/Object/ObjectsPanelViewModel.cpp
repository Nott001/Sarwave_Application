#include "Gui/ViewModels/Object/ObjectsPanelViewModel.hpp"

#include <algorithm>
#include <memory>

namespace Sarwave::Core::Gui::ViewModels::Object {

using Integration::PeopleTracking::Association::TargetObject;

ObjectsPanelViewModel::ObjectsPanelViewModel(QObject* parent) : QAbstractListModel(parent) {
}

ObjectsPanelViewModel::ObjectsPanelViewModel(DesignModeTag, QObject* parent)
    : QAbstractListModel(parent) {
    addDesignItems();
}

ObjectsPanelViewModel::ObjectsPanelViewModel(Models::RadarData& data, QObject* parent)
    : QAbstractListModel(parent) {
    // Same hand-off as RadarGridViewModel: the targets are copied here, on the thread that called
    // RadarData::update, while they are stable; only that copy crosses to the UI thread.
    m_subscription = RadarDataSubscription(data, [this, &data] {
        std::vector<TargetValues> targets;
        for (const TargetObject* target : data.allTargetObjects()) {
            targets.push_back({target->objectId(),
                               target->state() == TargetObject::TargetState::Active,
                               target->confidenceLevel, target->pos, target->vel, target->acc,
                               target->minZ, target->maxZ});
        }
        m_targetsPost.post(std::move(targets));
    });
}

int ObjectsPanelViewModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : count();
}

QVariant ObjectsPanelViewModel::data(const QModelIndex& index, int role) const {
    if (role != ItemRole || !index.isValid() || index.row() >= count()) return {};
    return QVariant::fromValue(
        static_cast<QObject*>(m_items[static_cast<std::size_t>(index.row())]));
}

QHash<int, QByteArray> ObjectsPanelViewModel::roleNames() const {
    return {{ItemRole, "item"}};
}

ObjectItemViewModel* ObjectsPanelViewModel::tryGet(int objectId) const {
    const auto it = m_itemsById.find(objectId);
    return it == m_itemsById.end() ? nullptr : it->second;
}

ObjectItemViewModel* ObjectsPanelViewModel::getOrAdd(int objectId) {
    if (auto* item = tryGet(objectId)) return item;

    auto* item = new ObjectItemViewModel(objectId, this);
    const int row = count();
    beginInsertRows({}, row, row);
    m_itemsById.emplace(objectId, item);
    m_items.push_back(item);
    endInsertRows();
    emit countChanged();
    return item;
}

bool ObjectsPanelViewModel::remove(int objectId) {
    const auto it = m_itemsById.find(objectId);
    if (it == m_itemsById.end()) return false;

    ObjectItemViewModel* item = it->second;
    const auto row =
        static_cast<int>(std::find(m_items.begin(), m_items.end(), item) - m_items.begin());
    beginRemoveRows({}, row, row);
    m_itemsById.erase(it);
    m_items.erase(m_items.begin() + row);
    endRemoveRows();
    item->deleteLater();  // a delegate may still be bound to it until the view catches up
    emit countChanged();
    return true;
}

void ObjectsPanelViewModel::clear() {
    if (m_items.empty()) return;

    beginResetModel();
    for (auto* item : m_items) item->deleteLater();
    m_itemsById.clear();
    m_items.clear();
    endResetModel();
    emit countChanged();
}

void ObjectsPanelViewModel::apply(const std::vector<TargetValues>& targets) {
    m_presentScratch.clear();
    for (const TargetValues& target : targets) {
        m_presentScratch.insert(target.objectId);

        ObjectItemViewModel* item = getOrAdd(target.objectId);
        item->setIsActive(target.isActive);
        item->setConfidenceLevel(target.confidenceLevel);
        item->setPosition(target.position);
        item->setVelocity(target.velocity);
        item->setAcceleration(target.acceleration);
        item->setMinZ(target.minZ);
        item->setMaxZ(target.maxZ);
    }

    if (m_items.size() == targets.size()) return;  // nothing left over to remove

    for (auto i = static_cast<std::ptrdiff_t>(m_items.size()) - 1; i >= 0; i--) {
        const int objectId = m_items[static_cast<std::size_t>(i)]->objectId();
        if (!m_presentScratch.contains(objectId)) remove(objectId);
    }
}

void ObjectsPanelViewModel::addDesignItems() {
    struct Sample {
        int id;
        float confidence;
        float minZ;
        float maxZ;
        bool active;
    };
    const Sample samples[] = {
        {0, 0.92f, 0.10f, 1.71f, true},
        {1, 0.64f, 0.08f, 1.12f, true},
        {2, 0.31f, 0.05f, 0.48f, false},
        {3, 0.87f, 0.12f, 1.68f, true},
    };

    for (const Sample& sample : samples) {
        ObjectItemViewModel* item = getOrAdd(sample.id);
        item->setConfidenceLevel(sample.confidence);
        item->setMinZ(sample.minZ);
        item->setMaxZ(sample.maxZ);
        item->setIsActive(sample.active);
    }
}

}  // namespace Sarwave::Core::Gui::ViewModels::Object
