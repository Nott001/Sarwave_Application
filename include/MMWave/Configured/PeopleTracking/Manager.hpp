//
// Created by lilacsyringa on 9/24/26.
//

#pragma once

#include <chrono>
#include <stdexcept>
#include <vector>
#include <deque>
#include <cstring>
#include <optional>

#include "../../Streaming.hpp"
#include "../../Tlv/TlvCore.hpp"
#include "../../Tlv/TlvOutput.hpp"
#include "../../Tlv/TlvPointCloud.hpp"
#include "EntryDecompiler.hpp"
#include "TargetObject.hpp"

namespace MMWave::Configured::PeopleTracking {
struct Manager {
    public:
        struct Entry {
            Entry(const Tlv::PointUnit unit, const std::chrono::steady_clock::time_point time) :
            unit(unit), time(time) {}

            Tlv::PointUnit unit;
            std::chrono::steady_clock::time_point time;
        };

    private:
        struct FullEntry {
            FullEntry(
                const std::vector<Tlv::CompressedPoint>& points,
                const Tlv::PointUnit unit,
                const std::chrono::steady_clock::time_point time) :
            points(points), unit(unit), time(time) {}

            std::vector<Tlv::CompressedPoint> points;
            Tlv::PointUnit unit;
            std::chrono::steady_clock::time_point time;
        };

        std::optional<FullEntry> last_entry;
        bool presence_detected = false;

        TargetObject non_objects;
        ObjectTracker object_tracker;
        std::deque<Entry> entry_queue;

    public:
        std::chrono::steady_clock::duration timespan;
        const std::deque<Entry>& getEntryQueue() const { return entry_queue; }

        void updateFrameData(const Streaming::Frame& frame,
            const std::chrono::steady_clock::time_point& time) {
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
                    if (object_tracker.instantiate(track.targetID)) {
                        IdentifiedObject* obj = object_tracker.getObject(track.targetID);
                        if (obj) obj->offset = entry_queue.size();
                    }
                    IdentifiedObject* obj = object_tracker.instantiateAndOrGet(track.targetID);
                    if (!obj) throw std::runtime_error("Failed to instantiate object with id=" + std::to_string(track.targetID));
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
                        target_ids.push_back(track.targetID); // guaranteed unique per track
                    }
                    target_ids.push_back(255);

                    for (const uint32_t id : target_ids) {
                        std::vector<Tlv::CompressedPoint> cloud_points;
                        size_t i = 0;
                        for (const uint8_t idx : indexes) {
                            if (static_cast<uint32_t>(idx) != id) continue;
                            cloud_points.push_back(points[i]);
                            i++;
                        }

                        TargetObject* obj;
                        if (id == 255) obj = &non_objects;
                        else obj = object_tracker.getObject(id);

                        if (obj->point_clouds.size() + obj->offset != entry_queue.size()) {
                            throw std::runtime_error(
                                "TargetObject id=" + std::to_string(id) +
                                (id == 255 ? " non-object" : "") +
                                " point_clouds.size()+offset != entry_queue.size()"
                            );
                        }
                        obj->updateCloud(cloud_points);
                    }
                    entry_queue.emplace_back(last_entry_value.unit, last_entry_value.time);
                }
            }
            if (data.presence.has_value()) presence_detected = data.presence.value();
        }

    private:
        void updatePointCloudData(Tlv::PointCloud::PointCloudRange point_cloud, const std::chrono::steady_clock::time_point& time) {
            const auto [payload, length] = point_cloud;
            if (length < sizeof(Tlv::PointUnit)) return;

            std::vector<Tlv::CompressedPoint> points;
            const size_t point_count = (length - sizeof(Tlv::PointUnit)) / sizeof(Tlv::CompressedPoint);
            points.resize(point_count);
            Tlv::PointUnit unit{};
            std::memcpy(&unit, payload, sizeof(Tlv::PointUnit));
            std::memcpy(
                points.data(), payload + sizeof(Tlv::PointUnit),
                point_count * sizeof(Tlv::CompressedPoint));

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

        void managePointCloudTimespan(const std::chrono::steady_clock::time_point& time) {
            const auto drop_time = time - timespan;
            std::optional<size_t> length = std::nullopt;
            for (const auto& entry : entry_queue) {
                if (entry.time >= drop_time) break;

                size_t new_length =  length.value_or(0) + 1;
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
}
