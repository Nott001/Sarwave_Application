#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/ConfigEnum.hpp"
#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

/// `lowPower`: ADC power mode. Cannot change between sensorStop and sensorStart without a reboot.
struct LowPower final : ConfigRecord<LowPower> {
    /// ADC power modes.
    enum class Mode : int {
        Regular = 0,
        /// Not supported on xWR6xxx devices.
        LowPower = 1,
    };

    static constexpr std::string_view Name = "lowPower";

    /// Unused; set to 0.
    int dontCare;
    /// ADC power mode.
    Mode adcMode;

    LowPower(int dontCare, Mode adcMode) : dontCare(dontCare), adcMode(adcMode) {
    }

    static ConfigParameterPtr parse(ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const LowPower&) const = default;

    friend const EnumDefinition<Mode>& enumDefinition(Mode);
};

}  // namespace Sarwave::Core::MMWave::Config::Parameters
