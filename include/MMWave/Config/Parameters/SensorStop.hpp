#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

/// `sensorStop`: Stops the sensor. Must come before FlushCfg and any reconfiguration.
struct SensorStop final : ConfigRecord<SensorStop> {
    static constexpr std::string_view Name = "sensorStop";

    static ConfigParameterPtr parse(ConfigArguments& /*args*/) {
        return std::make_shared<const SensorStop>();
    }

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override {
        return std::string(Name);
    }

    bool operator==(const SensorStop&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Config::Parameters
