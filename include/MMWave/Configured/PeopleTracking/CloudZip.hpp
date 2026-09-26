#pragma once

#include <tuple>
#include <vector>

#include "Manager.hpp"
#include "TargetObject.hpp"

namespace MMWave::Configured::PeopleTracking {

// Contains a pointer to each point_cloud paired with pointers to the
// corresponding entry_queue entry (at index + offset), accounting for offset.
// Pointers are invalidated if point_clouds or entry_queue are resized.
struct CloudEntry {
    const std::vector<CompressedPoint>* cloud;
    const PointUnit* unit;
    const std::chrono::steady_clock::time_point* time;

    [[nodiscard]] auto get() const
    {
        return std::tie(*cloud, *unit, *time);
    }
};

// Iterates over TargetObject.point_clouds and pairs each with the corresponding
// Manager entry_queue entry at index (i + offset), accounting for the offset.
// Requires the invariant point_clouds.size() + offset == entry_queue.size().
class CloudRange {
        TargetObject& target;
        Manager& manager;

    public:
        class Iterator {
                TargetObject* target;
                Manager* manager;
                size_t index;

            public:
                using iterator_category = std::forward_iterator_tag;
                using value_type = CloudEntry;
                using difference_type = std::ptrdiff_t;
                using pointer = CloudEntry*;
                using reference = CloudEntry&;

                Iterator(TargetObject* t, Manager* m, const size_t idx)
                    : target(t), manager(m), index(idx)
                {
                }

                CloudEntry operator*() const
                {
                    const size_t entry_index = index + target->offset;
                    return CloudEntry{&target->point_clouds[index],
                                      &manager->getEntryQueue()[entry_index].unit,
                                      &manager->getEntryQueue()[entry_index].time};
                }

                Iterator& operator++()
                {
                    ++index;
                    return *this;
                }

                bool operator!=(const Iterator& other) const
                {
                    return index != other.index;
                }
        };

        CloudRange(TargetObject& t, Manager& m) : target(t), manager(m)
        {
        }

        [[nodiscard]] Iterator begin() const
        {
            return {&target, &manager, 0};
        }
        [[nodiscard]] Iterator end() const
        {
            return {&target, &manager, target.point_clouds.size()};
        }
};

inline CloudRange pairClouds(TargetObject& target, Manager& manager)
{
    return {target, manager};
}

}
