#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `trackingCfg`: Enables the group tracker, sizes its memory, and tells it the velocity limits
/// and frame time of the chirp configuration it is fed by.
struct TrackingCfg final : MMWave::Config::ConfigRecord<TrackingCfg> {
    static constexpr std::string_view Name = "trackingCfg";

    /// Whether the group tracker runs.
    bool enable;
    /// Index of the built-in parameter set the tracker starts from; the other tracker
    /// commands override whatever that set contains.
    int initialConfigParams;
    /// Most detection points per frame the tracker accepts (up to 1000).
    int maxNumPoints;
    /// Most targets tracked at once (up to 200).
    int maxNumTracks;
    /// Largest radial velocity the detection layer reports, in units of 0.1 m/s (46 = 4.6 m/s).
    int maxRadialVelocity;
    /// Radial velocity resolution of the chirp configuration, in millimetres per second.
    int radialVelocityResolution;
    /// Frame time in milliseconds; should match `frameCfg`'s periodicity.
    int deltaT;
    /// Optional eighth argument: whether boresight filtering is on. Only used for ceiling
    /// mount. Nullopt when the line stops after `deltaT`, as the wall-mount configs do.
    std::optional<bool> boresightFilteringEnable;

    TrackingCfg(bool enable, int initialConfigParams, int maxNumPoints, int maxNumTracks,
                int maxRadialVelocity, int radialVelocityResolution, int deltaT,
                std::optional<bool> boresightFilteringEnable = std::nullopt)
        : enable(enable),
          initialConfigParams(initialConfigParams),
          maxNumPoints(maxNumPoints),
          maxNumTracks(maxNumTracks),
          maxRadialVelocity(maxRadialVelocity),
          radialVelocityResolution(radialVelocityResolution),
          deltaT(deltaT),
          boresightFilteringEnable(boresightFilteringEnable) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const TrackingCfg&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
