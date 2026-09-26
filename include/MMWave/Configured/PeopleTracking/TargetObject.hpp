#pragma once

#include <chrono>
#include <vector>
#include <deque>
#include <unordered_map>

#include "MMWave/Streaming.hpp"
#include "MMWave/TvlCore.hpp"
#include "MMWave/Configured/PeopleTracking/PointCloud.hpp"
#include "MMWave/Configured/PeopleTracking/TlvOutput.hpp"

namespace MMWave::Configured::PeopleTracking {
struct TargetObject {
    std::deque<std::vector<CompressedPoint>> point_clouds;
    size_t offset = 0;

    void updateCloud(const std::vector<CompressedPoint>& point_cloud) {
        point_clouds.push_back(point_cloud);
    }

    void dequeCloudEntries(const size_t range) {
        size_t remaining = range;
        if (offset > 0) {
            if (range >= offset) {
                remaining = range - offset;
                offset = 0;
            } else {
                remaining = 0;
                offset -= range;
            }
        }

        if (remaining >= point_clouds.size()) {
            point_clouds.clear();
        } else if (remaining > 0) {
            const auto start = point_clouds.begin();
            auto it = start;
            std::advance(it, static_cast<std::ptrdiff_t>(remaining));
            point_clouds.erase(start, it);
        }
    }
};

struct IdentifiedObject : TargetObject {
    //See TargetHeight
    float maxZ{}, minZ{};

    //See TargetList
    float posX{}, posY{}, posZ{};
    float velX{}, velY{}, velZ{};
    float accX{}, accY{}, accZ{};
    float ec[16]{};
    float g{};
    float confidenceLevel{};

    void updateTrackingData(const TargetList& tracked_target) {
        posX = tracked_target.posX;
        posY = tracked_target.posY;
        posZ = tracked_target.posZ;
        velX = tracked_target.velX;
        velY = tracked_target.velY;
        velZ = tracked_target.velZ;
        accX = tracked_target.accX;
        accY = tracked_target.accY;
        accZ = tracked_target.accZ;
        std::ranges::copy(tracked_target.ec, ec);
        g = tracked_target.g;
        confidenceLevel = tracked_target.confidenceLevel;
    }

    void updateHeightData(const TargetHeight& target_height) {
        maxZ = target_height.maxZ;
        minZ = target_height.minZ;
    }

    void updateData(
        const std::vector<CompressedPoint>& point_cloud,
        const TargetList& tracked_target,
        const TargetHeight& target_height)
    {
        updateCloud(point_cloud);
        updateTrackingData(tracked_target);
        updateHeightData(target_height);
    }
};

struct ObjectTracker {
    private:
        std::unordered_map<uint32_t, IdentifiedObject> objects;

    public:
        bool instantiate(const uint32_t id) {
            if (objects.contains(id)) return false;
            objects.emplace(id, IdentifiedObject{});
            return true;
        }

        IdentifiedObject* instantiateAndOrGet(const uint32_t id) {
            instantiate(id);
            return getObject(id);
        }

        IdentifiedObject* getObject(const uint32_t id) {
            auto it = objects.find(id);
            if (it == objects.end()) return nullptr;
            return &it->second;
        }

        [[nodiscard]] std::vector<uint32_t> getAllIds() const {
            std::vector<uint32_t> ids;
            ids.reserve(objects.size());
            for (const auto& [id, _] : objects) {
                ids.push_back(id);
            }
            return ids;
        }

        struct ObjectIterator {
            using iterator_category = std::forward_iterator_tag;
            using value_type = IdentifiedObject;
            using difference_type = std::ptrdiff_t;
            using pointer = IdentifiedObject*;
            using reference = IdentifiedObject&;

            std::unordered_map<uint32_t, IdentifiedObject>::iterator it;

            reference operator*() const { return it->second; }
            pointer operator->() const { return &it->second; }

            ObjectIterator& operator++() { ++it; return *this; }
            ObjectIterator operator++(int) { ObjectIterator tmp = *this; ++it; return tmp; }

            bool operator==(const ObjectIterator& other) const { return it == other.it; }
            bool operator!=(const ObjectIterator& other) const { return it != other.it; }
        };

        struct ObjectIteratorRange {
            std::unordered_map<uint32_t, IdentifiedObject>* objects;

            [[nodiscard]] ObjectIterator begin() const { return ObjectIterator(objects->begin()); }
            [[nodiscard]] ObjectIterator end() const { return ObjectIterator(objects->end()); }
        };

        ObjectIteratorRange getAllObjects() {
            return ObjectIteratorRange{&objects};
        }
};
}
