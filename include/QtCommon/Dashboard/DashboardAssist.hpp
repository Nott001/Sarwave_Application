#pragma once

#include <chrono>
#include <algorithm>
#include <limits>
#include <vector>
#include <cmath>

#include "MMWave/Configured/PeopleTracking/Processing/Manager.hpp"
#include "MMWave/Configured/PeopleTracking/PointCloud.hpp"

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
    static inline uint32_t s_hexColour;

    static void printObjects(
        MMWave::Configured::PeopleTracking::Processing::Manager& manager,
        const std::chrono::steady_clock::time_point& frames_start)
    {
        allocateColours(manager.getIdentifiedObjectCount());

        for (const auto& obj : manager.getAllIdentifiedObjects()) {
            changeColour();
            drawObject(obj, manager, frames_start);
        }

        setColor(0xFFFFFF);
        drawNonObjectPoints(manager, frames_start);
    }

private:
    [[nodiscard]] static uint32_t HsvToRgba(const Colour& colour) {
        const float h = colour.hue / 60.0f;
        const float s = colour.saturation;
        const float v = colour.value;

        const float c = v * s;
        const float x = c * (1.0f - std::fabs(std::fmod(h, 2.0f) - 1.0f));
        const float m = v - c;

        float r, g, b;

        if (h < 1.0f) {
            r = c; g = x; b = 0.0f;
        } else if (h < 2.0f) {
            r = x; g = c; b = 0.0f;
        } else if (h < 3.0f) {
            r = 0.0f; g = c; b = x;
        } else if (h < 4.0f) {
            r = 0.0f; g = x; b = c;
        } else if (h < 5.0f) {
            r = x; g = 0.0f; b = c;
        } else {
            r = c; g = 0.0f; b = x;
        }

        const auto toByte = [](const float value) -> uint32_t {
            return static_cast<uint32_t>(std::lround(value * 255.0f));
        };

        const uint32_t red   = toByte(r + m);
        const uint32_t green = toByte(g + m);
        const uint32_t blue  = toByte(b + m);
        const uint32_t alpha = toByte(colour.alpha);

        return (red << 24) |
               (green << 16) |
               (blue << 8) |
               alpha;
    }

    static void drawObject(
        const MMWave::Configured::PeopleTracking::Processing::TargetObject& obj,
        const MMWave::Configured::PeopleTracking::Processing::Manager& manager,
        const std::chrono::steady_clock::time_point& frame_start) {
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::lowest();

        for (size_t i = 0; i < obj.getSize(); i++) {
            auto const p = manager.getPointsOf(obj, i);
            for (auto const point : p.points) {
                const auto point_value =
                    MMWave::Configured::PeopleTracking::PointValue::Convert(point, p.unit);
                float alpha = static_cast<float>(
                    std::chrono::duration_cast<std::chrono::duration<double>>(
                        p.time - frame_start).count() / static_cast<double>(manager.timespan.count()));

                alpha = std::clamp(alpha, 0.0f, 1.0f);
                float x = point_value.range * std::sin(point_value.azimuth);
                float y = point_value.range * std::cos(point_value.azimuth);
                float z = point_value.range * std::sin(point_value.elevation);

                //drawPoint(...);

                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
                minZ = std::min(minZ, z);
                maxZ = std::max(maxZ, z);
            }
        }

        //drawBox(obj);
        //drawCentrePoint(obj);
        //drawVelocityArrow(obj);
    }

    static void drawNonObjectPoints(
    MMWave::Configured::PeopleTracking::Processing::Manager& manager,
    const std::chrono::steady_clock::time_point& frame_start)
    {
        const auto& non_obj = manager.getNonObject();
        for (size_t i = 0; i < non_obj.getSize(); i++) {
            auto const p = manager.getPointsOf(non_obj, i);
            for (auto const point : p.points) {
                const auto point_value =
                    MMWave::Configured::PeopleTracking::PointValue::Convert(point, p.unit);
                float alpha = static_cast<float>(
                    std::chrono::duration_cast<std::chrono::duration<double>>(
                        p.time - frame_start).count() /
                        static_cast<double>(manager.timespan.count()));

                alpha = std::clamp(alpha, 0.0f, 1.0f);
                const float x = point_value.range * std::sin(point_value.azimuth);
                const float y = point_value.range * std::cos(point_value.azimuth);
                const float z = point_value.range * std::sin(point_value.elevation);

                (void)x;
                (void)y;
                (void)z;

                //drawPoint(x, y, z, alpha);
            }
        }
    }

    static void allocateColours(const std::uint32_t count)
    {
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

    static void changeColour()
    {
        s_currentColourIndex = (s_currentColourIndex + 1) % s_colours.size();
        setColour(s_colours[s_currentColourIndex]);
    }

    static void setColour(const std::uint32_t hex_color) { s_hexColour = hex_color; }
};

} // namespace QtCommon::Dashboard
