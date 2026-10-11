#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `stateParam`: Thresholds that move a track between the DETECT, ACTIVE and FREE states, all in
/// frames.
struct StateParam final : MMWave::Config::ConfigRecord<StateParam> {
    static constexpr std::string_view Name = "stateParam";

    /// Consecutive hits for a DETECT track to become ACTIVE.
    int det2ActThre;
    /// Consecutive misses for a DETECT track to be freed.
    int det2FreeThre;
    /// Consecutive misses for a moving ACTIVE track to be freed.
    int active2FreeThre;
    /// Consecutive misses for a static track inside the static zone to be freed.
    int static2FreeThre;
    /// Consecutive misses for a track outside the static zone to be freed.
    int exit2FreeThre;
    /// Longest a track may stay static inside the static zone before it is freed.
    int sleep2FreeThre;

    StateParam(int det2ActThre, int det2FreeThre, int active2FreeThre, int static2FreeThre,
               int exit2FreeThre, int sleep2FreeThre)
        : det2ActThre(det2ActThre),
          det2FreeThre(det2FreeThre),
          active2FreeThre(active2FreeThre),
          static2FreeThre(static2FreeThre),
          exit2FreeThre(exit2FreeThre),
          sleep2FreeThre(sleep2FreeThre) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const StateParam&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
