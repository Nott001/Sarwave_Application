#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "MMWave/Config/ConfigEnum.hpp"
#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `antPhaseRot`: Phase rotation the board layout introduces on each virtual antenna: 1 for none,
/// -1 for a 180 degree rotation (an element fed from the opposite side).
struct AntPhaseRot final : MMWave::Config::ConfigRecord<AntPhaseRot> {
    /// What each value means.
    enum class Rotation : int {
        /// The element is fed from the opposite side: a 180 degree rotation.
        Inverted = -1,
        /// No rotation.
        None = 1,
    };

    static constexpr std::string_view Name = "antPhaseRot";

    /// One value per virtual antenna (12 for a 3 TX, 4 RX device).
    std::vector<Rotation> rotations;

    explicit AntPhaseRot(std::vector<Rotation> rotations) : rotations(std::move(rotations)) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const AntPhaseRot&) const = default;

    friend const MMWave::Config::EnumDefinition<Rotation>& enumDefinition(Rotation);
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
