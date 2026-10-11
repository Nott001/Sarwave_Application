#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/ConfigEnum.hpp"
#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

/// `channelCfg`: Enables receive and transmit antennas. Cannot change between sensorStop and
/// sensorStart without a reboot.
struct ChannelCfg final : ConfigRecord<ChannelCfg> {
    /// Receive antennas, as a bit mask.
    enum class Rx : int {
        None = 0,
        Rx0 = 1,
        Rx1 = 2,
        Rx2 = 4,
        Rx3 = 8,
    };

    /// Transmit antennas, as a bit mask. Also used by ChirpCfg.
    enum class Tx : int {
        None = 0,
        Tx0 = 1,
        Tx1 = 2,
        Tx2 = 4,
    };

    static constexpr std::string_view Name = "channelCfg";

    /// Receive antennas to enable; all four is 15 on the wire.
    Rx rxChannelEn;
    /// Transmit antennas to enable; all three is 7 on the wire.
    Tx txChannelEn;
    /// SoC cascading; not applicable, always 0.
    int cascading;

    ChannelCfg(Rx rxChannelEn, Tx txChannelEn, int cascading)
        : rxChannelEn(rxChannelEn), txChannelEn(txChannelEn), cascading(cascading) {
    }

    static ConfigParameterPtr parse(ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const ChannelCfg&) const = default;

    friend const EnumDefinition<Rx>& enumDefinition(Rx);

    friend const EnumDefinition<Tx>& enumDefinition(Tx);
};

}  // namespace Sarwave::Core::MMWave::Config::Parameters
