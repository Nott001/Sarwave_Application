#include "MMWave/Configured/PeopleTracking/Association/PointEntry.hpp"

#include <algorithm>
#include <numeric>

#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Association {

using Pointing::PointFigure;
using Pointing::PointValue;

PointEntry::PointEntry(const Tlv::TlvPointCloud& pointCloud, TimeSpan timestamp)
    : PointEntry(pointCloud, timestamp, Quaternion::identity()) {
}

PointEntry::PointEntry(const Tlv::TlvPointCloud& pointCloud, TimeSpan timestamp,
                       Quaternion leveling)
    : m_timestamp(timestamp) {
    const bool rotate = !leveling.isIdentity();
    m_pointFigures.reserve(static_cast<std::size_t>(pointCloud.points.count()));
    for (const auto& point : pointCloud.points) {
        PointFigure figure = PointValue::convert(point, pointCloud.unit).toFigure();
        if (rotate) figure.cartesian = transform(figure.cartesian, leveling);
        m_pointFigures.push_back(figure);
    }
}

PointEntry::PointEntry(std::vector<PointFigure> pointFigures, TimeSpan timestamp)
    : m_pointFigures(std::move(pointFigures)), m_timestamp(timestamp) {
}

std::span<const PointFigure> PointEntry::getPointsOf(int objectId) const {
    if (!m_relatedObject) throw InvalidOperationException("No such associated points.");

    const auto& related = *m_relatedObject;
    const auto it = std::lower_bound(related.begin(), related.end(), objectId,
                                     [](const auto& pair, int id) { return pair.first < id; });
    if (it == related.end() || it->first != objectId) return {};

    return std::span<const PointFigure>(m_pointFigures)
        .subspan(static_cast<std::size_t>(it->second.index),
                 static_cast<std::size_t>(it->second.length));
}

std::vector<int> PointEntry::objectIdsUsed(bool excludeConstants) const {
    std::vector<int> ids;
    if (!m_relatedObject) return ids;

    for (const auto& [objId, sector] : *m_relatedObject) {
        if (!excludeConstants || !isNonObjectPoint(objId)) ids.push_back(objId);
    }
    return ids;
}

void PointEntry::associatePoints(std::span<int> associatedObjectIds) {
    if (m_relatedObject) throw InvalidOperationException("Points are already associated.");

    if (m_pointFigures.size() != associatedObjectIds.size()) {
        throw ArgumentException("Their lengths do not match", "associatedObjectIdsBuffer");
    }

    // Sorts the ids ascending and applies the same reordering to the points.
    const std::size_t count = m_pointFigures.size();
    std::vector<std::size_t> order(count);
    std::iota(order.begin(), order.end(), std::size_t{0});
    std::stable_sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
        return associatedObjectIds[a] < associatedObjectIds[b];
    });

    std::vector<PointFigure> sortedPoints(count);
    std::vector<int> sortedIds(count);
    for (std::size_t i = 0; i < count; ++i) {
        sortedPoints[i] = m_pointFigures[order[i]];
        sortedIds[i] = associatedObjectIds[order[i]];
    }
    m_pointFigures = std::move(sortedPoints);
    std::copy(sortedIds.begin(), sortedIds.end(), associatedObjectIds.begin());

    std::vector<std::pair<int, ArraySector>> related;
    std::size_t start = 0;
    for (std::size_t i = 1; i <= count; ++i) {
        if (i != count && associatedObjectIds[i] == associatedObjectIds[start]) continue;

        related.emplace_back(associatedObjectIds[start],
                             ArraySector{static_cast<int>(start), static_cast<int>(i - start)});
        start = i;
    }

    m_relatedObject = std::move(related);
}

void PointEntryRange::unusedTargetObjects(std::span<const int> objectIds,
                                          std::vector<int>& unused) {
    m_usedScratch.clear();
    for (const auto& entry : m_pointsEntries) {
        for (int objId : entry->objectIdsUsed(true)) m_usedScratch.insert(objId);
    }

    unused.clear();
    for (int objId : objectIds) {
        if (m_usedScratch.contains(objId)) continue;
        unused.push_back(objId);
    }
}

void PointEntryRange::add(const Tlv::TlvPointCloud& pointCloud, TimeSpan timestamp,
                          Quaternion leveling) {
    auto entry = std::make_shared<PointEntry>(pointCloud, timestamp, leveling);
    m_pointsEntries.push_back(entry);
    m_last = std::move(entry);
}

void PointEntryRange::dropOldEntries(TimeSpan timestamp, TimeSpan duration) {
    const TimeSpan cutoff = timestamp - duration;

    while (!m_pointsEntries.empty() && m_pointsEntries.front()->timestamp() < cutoff) {
        m_pointsEntries.pop_front();
    }

    // The newest entry can only be dropped if everything was dropped.
    if (m_pointsEntries.empty()) m_last.reset();
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Association
