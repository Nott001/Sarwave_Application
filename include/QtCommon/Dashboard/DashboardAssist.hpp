#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <limits>
#include <vector>

#include "Dashboard/DashboardState.h"
#include "MMWave/Configured/PeopleTracking/PointCloud.hpp"
#include "MMWave/Configured/PeopleTracking/Processing/Manager.hpp"

namespace QtCommon::Dashboard {

struct Colour {
    float hue = 0.0f;
    float saturation = 1.0f;
    float value = 1.0f;
    float alpha = 1.0f;
};

class DashboardAssist {
   public:
    static inline std::vector<uint32_t> s_colours;
    static inline std::size_t s_currentColourIndex = 0;

    static void printObjects(MMWave::Configured::PeopleTracking::Processing::Manager& manager,
                             DashboardState& dashboard,
                             const std::chrono::nanoseconds& frames_start) {
        allocateColours(manager.getIdentifiedObjectCount());

        for (const auto& obj : manager.getAllIdentifiedObjects()) {
            uint32_t hex = changeColour();
            drawObject(obj, manager, frames_start, dashboard, hex);
        }

        drawPointOnly(manager, frames_start, dashboard);
    }

   private:
    static QString hexToQString(const std::uint32_t hex, const float alpha) {
        const uint8_t r = (hex >> 24) & 0xFF;
        const uint8_t g = (hex >> 16) & 0xFF;
        const uint8_t b = (hex >> 8) & 0xFF;
        const uint8_t origA = hex & 0xFF;
        const auto a = static_cast<uint8_t>(static_cast<float>(origA) * alpha);
        char buf[10];
        std::snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X", r, g, b, a);
        return QString::fromLatin1(buf);
    }

    [[nodiscard]] static uint32_t HsvToRgba(const Colour& colour) {
        const float h = colour.hue / 60.0f;
        const float s = colour.saturation;
        const float v = colour.value;

        const float c = v * s;
        const float x = c * (1.0f - std::fabs(std::fmod(h, 2.0f) - 1.0f));
        const float m = v - c;

        float r, g, b;

        if (h < 1.0f) {
            r = c;
            g = x;
            b = 0.0f;
        } else if (h < 2.0f) {
            r = x;
            g = c;
            b = 0.0f;
        } else if (h < 3.0f) {
            r = 0.0f;
            g = c;
            b = x;
        } else if (h < 4.0f) {
            r = 0.0f;
            g = x;
            b = c;
        } else if (h < 5.0f) {
            r = x;
            g = 0.0f;
            b = c;
        } else {
            r = c;
            g = 0.0f;
            b = x;
        }

        const auto toByte = [](const float value) -> uint32_t {
            return static_cast<uint32_t>(std::lround(value * 255.0f));
        };

        const uint32_t red = toByte(r + m);
        const uint32_t green = toByte(g + m);
        const uint32_t blue = toByte(b + m);
        const uint32_t alpha = toByte(colour.alpha);

        return (red << 24) | (green << 16) | (blue << 8) | alpha;
    }

    static float pointPercentage(const std::chrono::nanoseconds& frame_start,
                                 const std::chrono::nanoseconds& timestamp,
                                 const std::chrono::steady_clock::duration timespan) {
        const auto alpha = static_cast<float>(
            std::chrono::duration_cast<std::chrono::duration<double>>(frame_start - timestamp)
                .count() /
            static_cast<double>(timespan.count()));
        return std::clamp(alpha, 0.0f, 1.0f);
    }

    static void drawPointCloud(
        const MMWave::Configured::PeopleTracking::Processing::PointCompilation p,
        const std::chrono::nanoseconds& frame_start,
        const std::chrono::steady_clock::duration timespan, DashboardState& dashboard,
        const uint32_t hex) {
        const float alpha = pointPercentage(frame_start, p.time, timespan);
        auto colour = hexToQString(hex, alpha);

        for (auto const point : p.points) {
            const auto point_value =
                MMWave::Configured::PeopleTracking::PointValue::Convert(point, p.unit);
            const auto cartesian_point =
                MMWave::Configured::PeopleTracking::CartesianPoint::Convert(point_value);

            // PointCloud
        }
    }

    static void drawObject(
        const MMWave::Configured::PeopleTracking::Processing::IdentifiedObject& obj,
        const MMWave::Configured::PeopleTracking::Processing::Manager& manager,
        const std::chrono::nanoseconds& frame_start, DashboardState& dashboard,
        const uint32_t hex) {
        MMWave::Configured::PeopleTracking::CartesianPoint min = {
            std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max()};
        MMWave::Configured::PeopleTracking::CartesianPoint max = {
            std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest()};

        for (size_t i = 0; i < obj.getSize() - 1; ++i) {
            const auto p = manager.getPointsOf(obj, i);
            drawPointCloud(p, frame_start, manager.timespan, dashboard, hex);
        }

        {
            auto const p = manager.getPointsOf(obj, obj.getSize() - 1);
            const float alpha = pointPercentage(frame_start, p.time, manager.timespan);
            auto colour = hexToQString(hex, 1);

            for (auto const point : p.points) {
                const auto point_value =
                    MMWave::Configured::PeopleTracking::PointValue::Convert(point, p.unit);
                const auto cartesian_point =
                    MMWave::Configured::PeopleTracking::CartesianPoint::Convert(point_value);

                min.x = std::min(min.x, cartesian_point.x);
                min.y = std::min(min.y, cartesian_point.x);
                min.z = std::min(min.z, cartesian_point.y);
                max.x = std::max(max.x, cartesian_point.x);
                max.y = std::max(max.y, cartesian_point.x);
                max.z = std::max(max.z, cartesian_point.y);

                // PointCloud
            }
        }

        // Box
        // Circle
    }

    static void drawPointOnly(MMWave::Configured::PeopleTracking::Processing::Manager& manager,
                              const std::chrono::nanoseconds& frame_start,
                              DashboardState& dashboard) {
        const auto& non_obj = manager.getNonObject();

        for (size_t i = 0; i < non_obj.getSize(); ++i) {
            const auto p = manager.getPointsOf(non_obj, i);
            drawPointCloud(p, frame_start, manager.timespan, dashboard, 0xFFFFFF);
        }
    }

    static void allocateColours(const std::uint32_t count) {
        s_colours.clear();
        s_colours.reserve(count);
        const float hue_step = 360.0f / static_cast<float>(std::max(count, 1u));
        for (std::uint32_t i = 0; i < count; i++) {
            Colour clr = {hue_step * static_cast<float>(i), 1.0f, 1.0f, 1.0f};
            uint32_t rgba = HsvToRgba(clr);
            s_colours.push_back(rgba);
        }
        s_currentColourIndex = 0;
    }

    [[nodiscard]] static uint32_t changeColour() {
        s_currentColourIndex = (s_currentColourIndex + 1) % s_colours.size();
        return s_colours[s_currentColourIndex];
    }
};

}  // namespace QtCommon::Dashboard
