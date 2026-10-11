#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

/// `sensorStart`: Starts the sensor with the configuration sent so far. Normally the last line of a
/// .cfg file.
struct SensorStart final : ConfigRecord<SensorStart> {
    static constexpr std::string_view Name = "sensorStart";

    /// Optional. 0 restarts with the configuration the device already holds instead of
    /// applying a new one. Nullopt when the line has no argument, which is the usual form.
    std::optional<int> doReconfig;

    explicit SensorStart(std::optional<int> doReconfig = std::nullopt) : doReconfig(doReconfig) {
    }

    static ConfigParameterPtr parse(ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const SensorStart&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Config::Parameters
