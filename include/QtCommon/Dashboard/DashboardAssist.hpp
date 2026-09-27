#pragma once

#include <chrono>
#include <algorithm>
#include <limits>
#include <vector>
#include <cmath>
#include <cstdio>

#include "MMWave/Configured/PeopleTracking/Processing/Manager.hpp"
#include "MMWave/Configured/PeopleTracking/PointCloud.hpp"
#include "Dashboard/DashboardState.h"

namespace QtCommon::Dashboard {

constexpr float STANDARD_THICKNESS = 2.0;
constexpr float POINT_CLOUD_RADIUS = 0.05;
constexpr float CENTRAL_RADIUS = 0.03;
constexpr float BOX_PADDING = POINT_CLOUD_RADIUS;

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
        DashboardState& dashboard,
        const std::chrono::nanoseconds& frames_start)
    {
        allocateColours(manager.getIdentifiedObjectCount());
        dashboard.beginDrawings();

        for (const auto& obj : manager.getAllIdentifiedObjects()) {
            changeColour();
            drawObject(obj, manager, frames_start, dashboard);
        }

        setColour(0xFFFFFF);
        drawNonObjectPoints(manager, frames_start, dashboard);
        dashboard.endDrawings();
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
        const std::chrono::nanoseconds& frame_start,
        DashboardState& dashboard) {
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        //float minZ = std::numeric_limits<float>::max();
        //float maxZ = std::numeric_limits<float>::lowest();

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
                //float z = point_value.range * std::sin(point_value.elevation);

                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
                //minZ = std::min(minZ, z);
                //maxZ = std::max(maxZ, z);

                dashboard.addCircle(x, y, POINT_CLOUD_RADIUS, STANDARD_THICKNESS,
                    hexToQString(s_hexColour, alpha));
            }
        }

        const float boxMinX = minX - BOX_PADDING;
        const float boxMaxX = maxX + BOX_PADDING;
        const float boxMinY = minY - BOX_PADDING;
        const float boxMaxY = maxY + BOX_PADDING;

        dashboard.addLine(boxMinX, boxMinY, boxMaxX,
            boxMinY, STANDARD_THICKNESS, hexToQString(s_hexColour, 1.0f));
        dashboard.addLine(boxMaxX, boxMinY, boxMaxX,
            boxMaxY, STANDARD_THICKNESS, hexToQString(s_hexColour, 1.0f));
        dashboard.addLine(boxMaxX, boxMaxY, boxMinX,
            boxMaxY, STANDARD_THICKNESS, hexToQString(s_hexColour, 1.0f));
        dashboard.addLine(boxMinX, boxMaxY, boxMinX,
            boxMinY, STANDARD_THICKNESS, hexToQString(s_hexColour, 1.0f));

        const float cx = (minX + maxX) / 2.0f;
        const float cy = (minY + maxY) / 2.0f;
        dashboard.addCircle(cx, cy, CENTRAL_RADIUS, STANDARD_THICKNESS,
            hexToQString(s_hexColour, 1.0f), hexToQString(s_hexColour, 1.0f));
    }

    static void drawNonObjectPoints(
        MMWave::Configured::PeopleTracking::Processing::Manager& manager,
        const std::chrono::nanoseconds& frame_start,
        DashboardState& dashboard)
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
                //const float z = point_value.range * std::sin(point_value.elevation);

                dashboard.addCircle(x, y, POINT_CLOUD_RADIUS, STANDARD_THICKNESS,
                    hexToQString(s_hexColour, alpha));
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
