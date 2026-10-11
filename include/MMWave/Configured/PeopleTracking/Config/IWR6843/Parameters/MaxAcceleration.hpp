#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `maxAcceleration`: How much a target's acceleration is expected to change per frame; larger
/// values make the tracker trust its motion model less.
struct MaxAcceleration final : MMWave::Config::ConfigRecord<MaxAcceleration> {
    static constexpr std::string_view Name = "maxAcceleration";

    /// Lateral (X), in metres per second squared.
    float x;
    /// Longitudinal (Y), in metres per second squared.
    float y;
    /// Vertical (Z), in metres per second squared.
    float z;

    MaxAcceleration(float x, float y, float z) : x(x), y(y), z(z) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const MaxAcceleration&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
