#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `presenceBoundaryBox`: The occupancy area the presence indication reports on. In world
/// coordinates: origin on the floor directly below the sensor, Z = 0 at the floor.
struct PresenceBoundaryBox final : MMWave::Config::ConfigRecord<PresenceBoundaryBox> {
    static constexpr std::string_view Name = "presenceBoundaryBox";

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

    PresenceBoundaryBox(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
        : xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax), zMin(zMin), zMax(zMax) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const PresenceBoundaryBox&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
