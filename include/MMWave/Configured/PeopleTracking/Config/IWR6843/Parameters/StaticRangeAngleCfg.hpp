#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `staticRangeAngleCfg`: Whether static reflections are detected at all, and at what angular
/// resolution relative to the dynamic scene.
struct StaticRangeAngleCfg final : MMWave::Config::ConfigRecord<StaticRangeAngleCfg> {
    static constexpr std::string_view Name = "staticRangeAngleCfg";

    /// Always -1; subframes are not supported.
    int subFrameIdx;
    /// Whether static scene processing runs at all; the other fields are ignored when false.
    bool staticProcEnabled;
    /// Azimuth step as a multiple of the dynamic scene's azimuth search step.
    int staticAzimStepDeciFactor;
    /// Elevation step as a multiple of the dynamic scene's elevation search step.
    int staticElevStepDeciFactor;

    StaticRangeAngleCfg(int subFrameIdx, bool staticProcEnabled, int staticAzimStepDeciFactor,
                        int staticElevStepDeciFactor)
        : subFrameIdx(subFrameIdx),
          staticProcEnabled(staticProcEnabled),
          staticAzimStepDeciFactor(staticAzimStepDeciFactor),
          staticElevStepDeciFactor(staticElevStepDeciFactor) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const StaticRangeAngleCfg&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
