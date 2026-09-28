//
// Created by lilacsyringa on 9/24/26.
//

#pragma once

#include <chrono>
#include <cstring>
#include <deque>
#include <optional>
#include <stdexcept>
#include <vector>

#include "EntryDecompiler.hpp"
#include "MMWave/Configured/PeopleTracking/PointCloud.hpp"
#include "MMWave/Configured/PeopleTracking/TlvOutput.hpp"
#include "MMWave/Streaming.hpp"
#include "TargetObject.hpp"

/*
Root cause: In Manager::updateFrameData (Manager.hpp:84-157), the invariant
    point_clouds.size() + offset == entry_queue.size()
    breaks when a tracked object disappears from target.tracks and later reappears.
Here's the sequence:
1. Frame N: Object 0 is in target.tracks. entry_queue.emplace_back runs, growing entry_queue by 1.
    updateCloud also runs for object 0, growing its point_clouds by 1. Invariant holds.
2. Frame N+1: Object 0 disappears from target.tracks. target_ids (built from target.tracks + 255)
    does NOT include object 0's ID. So updateCloud is not called for object 0.
    But entry_queue.emplace_back still runs (because other tracks + 255 are present).
    entry_queue grows by 1; object 0's point_clouds does not. Invariant broken for object 0.
3. Frame N+2: Object 0 reappears in target.tracks.
    The check obj->getOffsetSize() != entry_queue.size() finds that object 0's
    point_clouds.size() + offset is now less than entry_queue.size() → throws std::runtime_error.
Fix needed: When instantiateAndOrGet finds an existing object whose state is inconsistent with the
    current entry_queue.size(), it should reset the object's point_clouds and offset to match the
    current queue size — essentially treating the reappearing object as a fresh tracking instance
    rather than carrying stale state from before it went missing.

To add more context, TargetList is a source of truth when it comes to tracking and counting targets.
    target.tracks's trackID is guaranteed to be unique, but not guaranteed to be in order.
The issue comes in when it identifies or reidentifies a new object, it actually requires a certain
    algorithm to reassociate from past targetIDs, or to identify if it's a new one.
There's also a missing implementation to drop IdentifiedTargets in Manager when that target is no
    longer being tracked.

That additional context clarifies the full picture. Here's the complete diagnosis:
Two missing pieces in Manager::updateFrameData:
1. No cleanup of stale targets. When a target ID disappears from target.tracks, it should be
    removed from object_tracker. Without this, the unordered_map accumulates orphaned
    IdentifiedObject instances whose point_clouds and offset never get updated (since updateCloud
    is only called for IDs in target.tracks), but entry_queue keeps growing. This is the direct
    cause of the invariant violation.
2. No reassociation handling. Because TargetList IDs are unique but not ordered,
    the radar can reassign a track ID to a different physical target across frames. When object 0
    is reidentified, the existing IdentifiedObject for ID=0 still has stale point_clouds from a
    completely different physical target. There's no logic to detect this and either reset or
    replace the stale object state.
What needs to happen: After processing target.tracks, Manager should remove any IdentifiedObject
    entries from object_tracker whose IDs are no longer present in the current target.tracks.
    This keeps point_clouds and entry_queue in sync for tracked objects, and when a reappeared
    (or reidentified) target comes back, it gets instantiated fresh with
    offset = entry_queue.size(), satisfying the invariant.
 */

namespace MMWave::Configured::PeopleTracking::Processing {
struct PointCompilation {
    const PointUnit unit;
    const std::chrono::nanoseconds time;
    const std::vector<CompressedPoint>& points;

    PointCompilation(const PointUnit& unit, const std::chrono::nanoseconds& time,
                     const std::vector<CompressedPoint>& points)
        : unit(unit), time(time), points(points) {}
};

struct Manager {
    explicit Manager(const std::chrono::steady_clock::duration timespan) :
    non_objects(0), timespan(timespan) {
    }

    struct Entry {
        Entry(const PointUnit unit, const std::chrono::nanoseconds time)
            : unit(unit), time(time) {
        }

        PointUnit unit;
        std::chrono::nanoseconds time;
    };

    private:
    struct FullEntry {
        FullEntry(const std::vector<CompressedPoint>& points, const PointUnit unit,
                  const std::chrono::nanoseconds time)
            : points(points), unit(unit), time(time) {
        }

        std::vector<CompressedPoint> points;
        PointUnit unit;
        std::chrono::nanoseconds time;
    };

    std::optional<FullEntry> last_entry;
    bool presence_detected = false;

    TargetObject non_objects;
    ObjectTracker object_tracker;
    std::deque<Entry> entry_queue;

    public:
    std::chrono::steady_clock::duration timespan;

    uint32_t getIdentifiedObjectCount() const {
        return object_tracker.getTargetCount();
    }

    const TargetObject& getNonObject() const {
        return non_objects;
    }

    ObjectTracker::ObjectIteratorRange getAllIdentifiedObjects() {
        return object_tracker.getAllObjects();
    }

    PointCompilation getPointsOf(const TargetObject& obj, const uint32_t i) const {
        const Entry& entry = entry_queue.at(i + obj.getOffset());
        return {entry.unit, entry.time, obj.getPointCloud(i)};
    }

    void updateFrameData(const Streaming::Frame& frame,
                         const std::chrono::nanoseconds& time) {
        const auto data = unpack(frame);

        // saved_last_entry captures the previous frame's point cloud data.
        // last_entry is reset to nullopt; if updatePointCloudData isn't called
        // this frame, it stays nullopt. If saved_last_entry is nullopt, the
        // target block is skipped entirely — point_clouds and entry_queue aren't
        // updated, but managePointCloudTimespan already removed old entries from
        // both, so the invariant point_clouds.size()+offset==entry_queue.size()
        // is preserved for the next iteration.
        const auto saved_last_entry = last_entry;
        last_entry = std::nullopt;

        managePointCloudTimespan(time);

        if (data.point_cloud.has_value()) updatePointCloudData(data.point_cloud.value(), time);
        if (data.target.has_value()) {
            const auto target = data.target.value();

            for (const auto& track : target.tracks) {
                IdentifiedObject* obj = object_tracker.instantiateAndOrGet(
                    track.targetID,
                    entry_queue.size());
                if (!obj)
                    throw std::runtime_error("Failed to instantiate object with id=" +
                                             std::to_string(track.targetID));
                obj->updateTrackingData(track);
            }

            for (const auto& height : target.heights) {
                IdentifiedObject* obj = object_tracker.getObject(height.targetID);
                obj->updateHeightData(height);
            }

            if (target.indexes.has_value() && saved_last_entry.has_value()) {
                const auto& last_entry_value = saved_last_entry.value();
                const auto& points = last_entry_value.points;
                const auto& indexes = target.indexes.value();

                std::vector<uint32_t> target_ids;
                target_ids.reserve(target.tracks.getCount() + 1);
                for (const auto& track : target.tracks) {
                    target_ids.push_back(track.targetID);  // guaranteed unique per track
                }
                target_ids.push_back(255);

                for (const uint32_t id : target_ids) {
                    std::vector<CompressedPoint> cloud_points;
                    size_t i = 0;
                    for (const uint8_t idx : indexes) {
                        if (static_cast<uint32_t>(idx) != id) continue;
                        cloud_points.push_back(points[i]);
                        i++;
                    }

                    TargetObject* obj;
                    if (id == 255)
                        obj = &non_objects;
                    else
                        obj = object_tracker.getObject(id);

                    if (obj->getOffsetSize() != entry_queue.size()) {
                        throw std::runtime_error("TargetObject id=" + std::to_string(id) +
                                                 (id == 255 ? " non-object" : "") +
                                                 " point_clouds.size()+offset!=entry_queue.size()");
                    }
                    obj->updateCloud(cloud_points);
                }
                entry_queue.emplace_back(last_entry_value.unit, last_entry_value.time);
            }
        }
        if (data.presence.has_value()) presence_detected = data.presence.value();
    }

   private:
    void updatePointCloudData(PointCloud::PointCloudRange point_cloud,
                              const std::chrono::nanoseconds& time) {
        const auto [payload, length] = point_cloud;
        if (length < sizeof(PointUnit)) return;

        std::vector<CompressedPoint> points;
        const size_t point_count = (length - sizeof(PointUnit)) / sizeof(CompressedPoint);
        points.resize(point_count);
        PointUnit unit{};
        std::memcpy(&unit, payload, sizeof(PointUnit));
        std::memcpy(points.data(), payload + sizeof(PointUnit),
                    point_count * sizeof(CompressedPoint));

        last_entry = FullEntry(points, unit, time);
    }

    void dequeTimestampEntries(const size_t range) {
        const auto size = entry_queue.size();
        if (range > size) return;

        const auto start = entry_queue.begin();
        auto it = start;
        std::advance(it, static_cast<std::ptrdiff_t>(range));
        entry_queue.erase(start, it);
    }

    void managePointCloudTimespan(const std::chrono::nanoseconds& time) {
        const auto drop_time = time - timespan;
        std::optional<size_t> length = std::nullopt;
        for (const auto& entry : entry_queue) {
            if (entry.time >= drop_time) break;

            size_t new_length = length.value_or(0) + 1;
            length = new_length;
        }

        if (!length.has_value()) return;
        const auto length_val = length.value();
        non_objects.dequeCloudEntries(length_val);
        for (IdentifiedObject& target_object : object_tracker.getAllObjects()) {
            target_object.dequeCloudEntries(length_val);
        }
        dequeTimestampEntries(length_val);
    }
};
}  // namespace MMWave::Configured::PeopleTracking
