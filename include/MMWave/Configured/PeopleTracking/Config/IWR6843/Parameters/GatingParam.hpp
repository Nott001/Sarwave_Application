#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `gatingParam`: The gate around each track that decides which points can be associated with it.
struct GatingParam final : MMWave::Config::ConfigRecord<GatingParam> {
    static constexpr std::string_view Name = "gatingParam";

    /// How far the gate can grow beyond the limits below; 3 follows the three-sigma rule.
    float gain;
    /// Gate limit in width (X), in metres.
    float limitWidth;
    /// Gate limit in depth (Y), in metres.
    float limitDepth;
    /// Gate limit in height (Z), in metres.
    float limitHeight;
    /// Gate limit in radial velocity, in metres per second.
    float limitVelocity;

    GatingParam(float gain, float limitWidth, float limitDepth, float limitHeight,
                float limitVelocity)
        : gain(gain),
          limitWidth(limitWidth),
          limitDepth(limitDepth),
          limitHeight(limitHeight),
          limitVelocity(limitVelocity) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const GatingParam&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
