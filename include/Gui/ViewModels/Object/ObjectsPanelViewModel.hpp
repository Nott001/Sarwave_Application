#pragma once

#include <QtQml/qqmlregistration.h>

#include <QAbstractListModel>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Gui/Models/RadarData.hpp"
#include "Gui/ViewModels/Object/ObjectItemViewModel.hpp"
#include "Gui/ViewModels/ViewModelBase.hpp"

namespace Sarwave::Core::Gui::ViewModels::Object {

/// The tracked targets shown in the objects panel: a list for the view to display (the model
/// rows, role "item"), plus an index from object id to its item so a target can be found and
/// updated without searching the list. The two are only ever changed together, through the
/// methods here.
///
/// The list drives the UI, so call these on the UI thread.
class ObjectsPanelViewModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by the dashboard view models.")

    Q_PROPERTY(int count READ count NOTIFY countChanged)

   public:
    enum Roles {
        ItemRole = Qt::UserRole + 1,
    };

    /// Starts empty.
    explicit ObjectsPanelViewModel(QObject* parent = nullptr);

    /// As in the previewer: a few sample targets.
    explicit ObjectsPanelViewModel(DesignModeTag, QObject* parent = nullptr);

    /// Follows `data`: one item per target, updated on every frame.
    explicit ObjectsPanelViewModel(Models::RadarData& data, QObject* parent = nullptr);

    // QAbstractListModel
    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    /// The items in the order their targets first appeared.
    [[nodiscard]] const std::vector<ObjectItemViewModel*>& items() const {
        return m_items;
    }

    [[nodiscard]] int count() const {
        return static_cast<int>(m_items.size());
    }

    [[nodiscard]] bool contains(int objectId) const {
        return m_itemsById.contains(objectId);
    }

    /// Null when there is no item for `objectId`.
    [[nodiscard]] ObjectItemViewModel* tryGet(int objectId) const;

    /// The item for `objectId`, added to the end of the list if it isn't there yet.
    ObjectItemViewModel* getOrAdd(int objectId);

    /// False if there was no item for `objectId`.
    bool remove(int objectId);

    void clear();

    /// One target's values as copied off the capture thread; see apply().
    struct TargetValues {
        int objectId;
        bool isActive;
        float confidenceLevel;
        MMWave::Vector3 position;
        MMWave::Vector3 velocity;
        MMWave::Vector3 acceleration;
        float minZ;
        float maxZ;
    };

    /// Brings the list in line with one frame's targets: new ones added, the rest updated,
    /// gone ones removed.
    void apply(const std::vector<TargetValues>& targets);

   signals:
    void countChanged();

   private:
    void addDesignItems();
    void postApply(std::vector<TargetValues> targets);

    std::vector<ObjectItemViewModel*> m_items;  // children of this object
    std::unordered_map<int, ObjectItemViewModel*> m_itemsById;
    std::unordered_set<int> m_presentScratch;  // reused by apply, not reallocated each frame

    RadarDataSubscription m_subscription;  // last: ended first
};

}  // namespace Sarwave::Core::Gui::ViewModels::Object
