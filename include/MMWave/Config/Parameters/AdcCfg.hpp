#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/ConfigEnum.hpp"
#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

/// `adcCfg`: ADC sample format. Cannot change between sensorStop and sensorStart without a reboot.
struct AdcCfg final : ConfigRecord<AdcCfg> {
    /// ADC sample widths.
    enum class BitDepth : int {
        Bits12 = 0,
        Bits14 = 1,
        Bits16 = 2,
    };

    /// ADC output formats.
    enum class OutputFormat : int {
        Real = 0,
        /// Complex, image band filtered out.
        Complex1x = 1,
        /// Complex, image band visible.
        Complex2x = 2,
    };

    static constexpr std::string_view Name = "adcCfg";

    /// ADC sample width. Only 16 bit is supported.
    BitDepth numAdcBits;
    /// Real or complex samples. People tracking supports only complex 1x.
    OutputFormat adcOutputFmt;

    AdcCfg(BitDepth numAdcBits, OutputFormat adcOutputFmt)
        : numAdcBits(numAdcBits), adcOutputFmt(adcOutputFmt) {
    }

    static ConfigParameterPtr parse(ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const AdcCfg&) const = default;

    friend const EnumDefinition<BitDepth>& enumDefinition(BitDepth);

    friend const EnumDefinition<OutputFormat>& enumDefinition(OutputFormat);
};

}  // namespace Sarwave::Core::MMWave::Config::Parameters
