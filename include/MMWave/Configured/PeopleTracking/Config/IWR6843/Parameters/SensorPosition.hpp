#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `sensorPosition`: Where the sensor is mounted and how it is turned, so the tracker can convert
/// sensor-relative points to world coordinates.
struct SensorPosition final : MMWave::Config::ConfigRecord<SensorPosition> {
    static constexpr std::string_view Name = "sensorPosition";

    /// Height of the sensor above the floor, in metres.
    float sensorHeight;
    /// Rotation about the vertical axis in degrees; positive turns from +X towards +Y.
    float azimTilt;
    /// Downward tilt in degrees; 90 for a ceiling mount looking straight down.
    float elevTilt;

    SensorPosition(float sensorHeight, float azimTilt, float elevTilt)
        : sensorHeight(sensorHeight), azimTilt(azimTilt), elevTilt(elevTilt) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const SensorPosition&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
