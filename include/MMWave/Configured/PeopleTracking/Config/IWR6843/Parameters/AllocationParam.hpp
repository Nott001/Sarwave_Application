#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `allocationParam`: What a cluster of unassociated points must look like to become a new track.
struct AllocationParam final : MMWave::Config::ConfigRecord<AllocationParam> {
    static constexpr std::string_view Name = "allocationParam";

    /// Minimum total SNR of the cluster.
    float snrThre;
    /// Minimum total SNR when the cluster is behind an existing track.
    float snrThreObscured;
    /// Minimum radial velocity of the cluster's centroid, in metres per second.
    float velocityThre;
    /// Minimum number of points in the cluster.
    float pointsThre;
    /// Largest squared distance from the centroid for a point to join, in square metres.
    float maxDistanceThre;
    /// Largest radial velocity difference from the centroid for a point to join, in metres per
    /// second.
    float maxVelThre;

    AllocationParam(float snrThre, float snrThreObscured, float velocityThre, float pointsThre,
                    float maxDistanceThre, float maxVelThre)
        : snrThre(snrThre),
          snrThreObscured(snrThreObscured),
          velocityThre(velocityThre),
          pointsThre(pointsThre),
          maxDistanceThre(maxDistanceThre),
          maxVelThre(maxVelThre) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const AllocationParam&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
