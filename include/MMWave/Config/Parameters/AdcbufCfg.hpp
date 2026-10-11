#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/ConfigEnum.hpp"
#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

/// `adcbufCfg`: ADC buffer hardware configuration.
struct AdcbufCfg final : ConfigRecord<AdcbufCfg> {
    /// Sample formats in the buffer. Note the numbering is the reverse of AdcCfg::OutputFormat.
    enum class OutputFormat : int {
        Complex = 0,
        Real = 1,
    };

    /// Which half of a sample holds I and which holds Q.
    enum class IqOrder : int {
        /// I in the least significant half, Q in the most significant.
        ILsbQMsb = 0,
        /// Q in the least significant half, I in the most significant. The only option the demos
        /// support.
        QLsbIMsb = 1,
    };

    /// How the receive channels are arranged in the buffer.
    enum class ChannelLayout : int {
        /// Only supported on xWR14xx.
        Interleaved = 0,
        NonInterleaved = 1,
    };

    static constexpr std::string_view Name = "adcbufCfg";

    /// Subframe this applies to; -1 for legacy (non-advanced) frame mode.
    int subFrameIdx;
    /// Format of the samples placed in the buffer.
    OutputFormat adcOutputFmt;
    /// Which half of each sample holds I and which holds Q.
    IqOrder sampleSwap;
    /// Whether the receive channels are interleaved in the buffer.
    ChannelLayout chanInterleave;
    /// Chirps per ping/pong buffer switch; 1 for chains that use the HWA for the range FFT.
    int chirpThreshold;

    AdcbufCfg(int subFrameIdx, OutputFormat adcOutputFmt, IqOrder sampleSwap,
              ChannelLayout chanInterleave, int chirpThreshold)
        : subFrameIdx(subFrameIdx),
          adcOutputFmt(adcOutputFmt),
          sampleSwap(sampleSwap),
          chanInterleave(chanInterleave),
          chirpThreshold(chirpThreshold) {
    }

    static ConfigParameterPtr parse(ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const AdcbufCfg&) const = default;

    friend const EnumDefinition<OutputFormat>& enumDefinition(OutputFormat);

    friend const EnumDefinition<IqOrder>& enumDefinition(IqOrder);

    friend const EnumDefinition<ChannelLayout>& enumDefinition(ChannelLayout);
};

}  // namespace Sarwave::Core::MMWave::Config::Parameters
