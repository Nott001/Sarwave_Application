#pragma once

#include <QMetaType>
#include <memory>
#include <vector>

#include "Integration/PeopleTracking/Association/TargetObject.hpp"
#include "MMWave/Common/Numerics.hpp"

namespace Sarwave::Core::Gui::ViewModels::Radar {

/// One tracked target at the moment the snapshot was taken. Position in metres.
struct TargetSnapshot {
    int objectId = 0;
    bool isActive = false;
    MMWave::Vector3 position;

    friend bool operator==(const TargetSnapshot&, const TargetSnapshot&) = default;
};

/// The points of one frame that share an owner.
struct PointGroupSnapshot {
    /// Zero or above = the target these points belong to; negative = a NonObjectPoint value:
    /// the reason the tracker rejected them, or NonObjectPoint::PointNotYetAssigned while
    /// their frame has not been through the tracker yet.
    int objectId = 0;
    std::vector<MMWave::Vector3> points;

    [[nodiscard]] bool isUnassigned() const {
        return objectId ==
               static_cast<int>(
                   Integration::PeopleTracking::Association::NonObjectPoint::PointNotYetAssigned);
    }
};

/// The points of one frame, with how far through their lifetime they are.
struct PointCloudSnapshot {
    /// (current time - frame timestamp) / retention duration, as of the snapshot:
    /// 0 = just arrived, 1 = about to be dropped.
    float fade = 0;

    /// Rejected points first, then one group per target.
    std::vector<PointGroupSnapshot> groups;
};

/// Immutable copy of what the radar grid should draw. Built once per frame from RadarData, so the
/// view never reads the manager's live collections. Coordinates: X = lateral metres (right +),
/// Y = forward metres, Z = height.
class RadarSnapshot {
   public:
    RadarSnapshot() = default;
    RadarSnapshot(std::vector<TargetSnapshot> targets, std::vector<PointCloudSnapshot> pointClouds)
        : m_targets(std::move(targets)), m_pointClouds(std::move(pointClouds)) {
    }

    static const std::shared_ptr<const RadarSnapshot>& empty();

    [[nodiscard]] const std::vector<TargetSnapshot>& targets() const {
        return m_targets;
    }

    /// Oldest first.
    [[nodiscard]] const std::vector<PointCloudSnapshot>& pointClouds() const {
        return m_pointClouds;
    }

   private:
    std::vector<TargetSnapshot> m_targets;
    std::vector<PointCloudSnapshot> m_pointClouds;
};

/// How a snapshot travels between the view model and RadarDataOverlay (a QML property value):
/// shared and never changed, so handing it over copies nothing.
using RadarSnapshotPtr = std::shared_ptr<const RadarSnapshot>;

}  // namespace Sarwave::Core::Gui::ViewModels::Radar

Q_DECLARE_METATYPE(Sarwave::Core::Gui::ViewModels::Radar::RadarSnapshotPtr)
