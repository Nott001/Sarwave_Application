#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

/// `compRangeBiasAndRxChanPhase`: Corrects a constant offset in measured range and the gain and
/// phase differences between the virtual receive channels. The values come from the SDK's
/// calibration procedure; "0" followed by "1 0" for every channel means no correction.
struct CompRangeBiasAndRxChanPhase final : ConfigRecord<CompRangeBiasAndRxChanPhase> {
    /// One complex correction factor (the C# `(float Re, float Im)` tuple).
    struct PhaseFactor {
        float re;
        float im;

        bool operator==(const PhaseFactor&) const = default;
    };

    static constexpr std::string_view Name = "compRangeBiasAndRxChanPhase";

    /// Range estimation bias to remove, in metres. Unused by the people tracking chain.
    float rangeBias;
    /// One complex factor per virtual antenna, in the order TX0-RX0, TX0-RX1, ... TX2-RX3
    /// (12 for a 3 TX, 4 RX device).
    std::vector<PhaseFactor> rxChanPhase;

    CompRangeBiasAndRxChanPhase(float rangeBias, std::vector<PhaseFactor> rxChanPhase)
        : rangeBias(rangeBias), rxChanPhase(std::move(rxChanPhase)) {
    }

    static ConfigParameterPtr parse(ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const CompRangeBiasAndRxChanPhase&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Config::Parameters
