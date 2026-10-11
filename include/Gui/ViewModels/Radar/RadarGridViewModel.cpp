#include "Gui/ViewModels/Radar/RadarGridViewModel.hpp"

#include <algorithm>

#include "Gui/Common/DotNetRandom.hpp"

namespace Sarwave::Core::Gui::ViewModels::Radar {
namespace {

using MMWave::Vector3;
using MMWave::Configured::PeopleTracking::Association::NonObjectPoint;
using MMWave::Configured::PeopleTracking::Association::TargetObject;

std::vector<Vector3> toVectors(
    std::span<const MMWave::Configured::PeopleTracking::Pointing::PointFigure> values) {
    std::vector<Vector3> points;
    points.reserve(values.size());
    for (const auto& value : values) points.push_back(value.cartesian);
    return points;
}

std::vector<Vector3> scatter(Common::DotNetRandom& random, Vector3 centre, float spread,
                             int count) {
    std::vector<Vector3> points;
    points.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; i++) {
        // Drawn in the order C# evaluates the constructor arguments: x, then y.
        const float x = (random.nextSingle() - 0.5f) * spread;
        const float y = (random.nextSingle() - 0.5f) * spread;
        points.push_back(centre + Vector3{x, y, 0});
    }
    return points;
}

}  // namespace

RadarGridViewModel::RadarGridViewModel(QObject* parent) : ViewModelBase(parent) {
}

RadarGridViewModel::RadarGridViewModel(DesignModeTag, QObject* parent) : ViewModelBase(parent) {
    m_snapshot = std::make_shared<const RadarSnapshot>(designSnapshot());
}

RadarGridViewModel::RadarGridViewModel(Models::RadarData& data, QObject* parent)
    : ViewModelBase(parent) {
    // The snapshot is taken here, on the thread that called RadarData::update, while the data is
    // stable; only the finished copy crosses to the UI thread.
    m_subscription = RadarDataSubscription(data, [this, &data] {
        auto snapshot = std::make_shared<const RadarSnapshot>(createSnapshot(data));
        onUiThread([this, snapshot] { setSnapshot(snapshot); });
    });
}

void RadarGridViewModel::setSnapshot(RadarSnapshotPtr value) {
    if (m_snapshot == value) return;
    m_snapshot = std::move(value);
    emit snapshotChanged();
}

#define SARWAVE_SETTER(Name, member, signal)      \
    void RadarGridViewModel::Name(double value) { \
        if (member == value) return;              \
        member = value;                           \
        emit signal();                            \
    }

SARWAVE_SETTER(setMinRange, m_minRange, minRangeChanged)
SARWAVE_SETTER(setMaxRange, m_maxRange, maxRangeChanged)
SARWAVE_SETTER(setRangeStep, m_rangeStep, rangeStepChanged)
SARWAVE_SETTER(setMinAzimuth, m_minAzimuth, minAzimuthChanged)
SARWAVE_SETTER(setMaxAzimuth, m_maxAzimuth, maxAzimuthChanged)
SARWAVE_SETTER(setAzimuthStep, m_azimuthStep, azimuthStepChanged)
#undef SARWAVE_SETTER

void RadarGridViewModel::setSensorLabel(const QString& value) {
    if (m_sensorLabel == value) return;
    m_sensorLabel = value;
    emit sensorLabelChanged();
}

RadarSnapshot RadarGridViewModel::createSnapshot(const Models::RadarData& data) {
    std::vector<TargetSnapshot> targets;
    for (const TargetObject* target : data.allTargetObjects()) {
        targets.push_back({target->objectId(), target->state() == TargetObject::TargetState::Active,
                           target->pos});
    }

    // "Current time" is the timestamp of the update being snapshotted.
    const MMWave::TimeSpan now = data.lastUpdate();
    const MMWave::TimeSpan duration = data.duration();

    std::vector<PointCloudSnapshot> pointClouds;
    for (const auto& entry : data.pointClouds()) {
        std::vector<PointGroupSnapshot> groups;
        if (!entry->isAssociated()) {
            groups.push_back({static_cast<int>(NonObjectPoint::PointNotYetAssigned),
                              toVectors(entry->pointValues())});
        } else {
            // An associated entry keeps its points sorted by owner, rejected ones first.
            for (int objectId : entry->objectIdsUsed(false)) {
                groups.push_back({objectId, toVectors(entry->getPointsOf(objectId))});
            }
        }

        const float fade = duration > MMWave::TimeSpan::zero()
                               ? std::clamp(static_cast<float>(MMWave::TimeSpans::ratio(
                                                now - entry->timestamp(), duration)),
                                            0.0f, 1.0f)
                               : 0.0f;
        pointClouds.push_back({fade, std::move(groups)});
    }

    return {std::move(targets), std::move(pointClouds)};
}

RadarSnapshot RadarGridViewModel::designSnapshot() {
    const std::vector<TargetSnapshot> targets{
        {0, true, {-0.8f, 2.6f, 1.1f}},
        {1, true, {1.1f, 4.2f, 1.0f}},
        {2, false, {0.3f, 1.4f, 0.9f}},
    };

    // Five frames, oldest first, so the previewer shows the fade from old to new. The newest one
    // is still unassigned, as it would be live; the others carry some rejected points too.
    Common::DotNetRandom random(1);
    std::vector<PointCloudSnapshot> pointClouds;
    for (int frame = 0; frame < 5; frame++) {
        const bool assigned = frame < 4;
        std::vector<PointGroupSnapshot> groups;
        if (assigned) {
            groups.push_back({static_cast<int>(NonObjectPoint::PointLikelyNoise),
                              scatter(random, {0.2f, 3.4f, 0}, 3.0f, 5)});
        }

        for (const TargetSnapshot& target : targets) {
            groups.push_back(
                {assigned ? target.objectId : static_cast<int>(NonObjectPoint::PointNotYetAssigned),
                 scatter(random, target.position, 0.6f, 6)});
        }

        pointClouds.push_back({0.8f - 0.2f * static_cast<float>(frame), std::move(groups)});
    }

    return {targets, std::move(pointClouds)};
}

}  // namespace Sarwave::Core::Gui::ViewModels::Radar
