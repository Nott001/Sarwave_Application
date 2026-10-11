#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `staticBoundaryBox`: The zone where targets may stay still for a long time: static points
/// outside it are ignored, and a track inside it survives far longer without detections. In world
/// coordinates: origin on the floor directly below the sensor, Z = 0 at the floor.
struct StaticBoundaryBox final : MMWave::Config::ConfigRecord<StaticBoundaryBox> {
    static constexpr std::string_view Name = "staticBoundaryBox";

    /// Lowest X (left-right), in metres.
    float xMin;
    /// Highest X (left-right), in metres.
    float xMax;
    /// Lowest Y (depth from the sensor), in metres.
    float yMin;
    /// Highest Y (depth from the sensor), in metres.
    float yMax;
    /// Lowest Z (height above the floor), in metres.
    float zMin;
    /// Highest Z (height above the floor), in metres.
    float zMax;

    StaticBoundaryBox(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
        : xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax), zMin(zMin), zMax(zMax) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const StaticBoundaryBox&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
