#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

/// `flushCfg`: Discards the configuration the device currently holds, so a new one can be sent.
/// Issued after SensorStop.
struct FlushCfg final : ConfigRecord<FlushCfg> {
    static constexpr std::string_view Name = "flushCfg";

    static ConfigParameterPtr parse(ConfigArguments& /*args*/) {
        return std::make_shared<const FlushCfg>();
    }

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override {
        return std::string(Name);
    }

    bool operator==(const FlushCfg&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Config::Parameters
