#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/ConfigEnum.hpp"
#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

/// `dfeDataOutputMode`: Selects how chirps are grouped into frames. Cannot change between
/// sensorStop and sensorStart without a reboot.
struct DfeDataOutputMode final : ConfigRecord<DfeDataOutputMode> {
    /// What each `modeType` number means.
    enum class Mode : int {
        /// Frame based chirps.
        FrameBased = 1,
        /// Continuous chirping.
        Continuous = 2,
        /// Advanced frame configuration (subframes).
        AdvancedFrame = 3,
    };

    static constexpr std::string_view Name = "dfeDataOutputMode";

    /// How chirps are grouped. People tracking supports only Mode::FrameBased.
    Mode modeType;

    explicit DfeDataOutputMode(Mode modeType) : modeType(modeType) {
    }

    static ConfigParameterPtr parse(ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const DfeDataOutputMode&) const = default;

    friend const EnumDefinition<Mode>& enumDefinition(Mode);
};

}  // namespace Sarwave::Core::MMWave::Config::Parameters
