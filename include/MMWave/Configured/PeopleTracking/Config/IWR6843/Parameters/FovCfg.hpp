#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `fovCfg`: Field of view. Nothing outside these angles is detected.
struct FovCfg final : MMWave::Config::ConfigRecord<FovCfg> {
    static constexpr std::string_view Name = "fovCfg";

    /// Always -1; subframes are not supported.
    int subFrameIdx;
    /// Azimuth limit in degrees to either side of boresight.
    float azimuthFoV;
    /// Elevation limit in degrees above and below boresight.
    float elevationFoV;

    FovCfg(int subFrameIdx, float azimuthFoV, float elevationFoV)
        : subFrameIdx(subFrameIdx), azimuthFoV(azimuthFoV), elevationFoV(elevationFoV) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const FovCfg&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
