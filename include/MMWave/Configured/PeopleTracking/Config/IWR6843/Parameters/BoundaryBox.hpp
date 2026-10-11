#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `boundaryBox`: The space the tracker works in; points outside it are discarded. In world
/// coordinates: origin on the floor directly below the sensor, Z = 0 at the floor.
struct BoundaryBox final : MMWave::Config::ConfigRecord<BoundaryBox> {
    static constexpr std::string_view Name = "boundaryBox";

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

    BoundaryBox(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
        : xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax), zMin(zMin), zMax(zMax) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const BoundaryBox&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
