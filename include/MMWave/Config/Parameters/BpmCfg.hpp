#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

/// `bpmCfg`: Binary phase modulation: lets two transmitters fire together with opposite phase
/// patterns for about 3 dB more signal. Optional; BPM is off when the line is absent.
struct BpmCfg final : ConfigRecord<BpmCfg> {
    static constexpr std::string_view Name = "bpmCfg";

    /// Subframe this applies to; -1 for legacy frame mode.
    int subFrameIdx;
    /// False = all transmitters time-multiplexed; true = BPM enabled.
    bool enabled;
    /// Chirp index of the first BPM chirp (both transmitters in phase).
    int chirp0Idx;
    /// Chirp index of the second BPM chirp (second transmitter inverted).
    int chirp1Idx;

    BpmCfg(int subFrameIdx, bool enabled, int chirp0Idx, int chirp1Idx)
        : subFrameIdx(subFrameIdx), enabled(enabled), chirp0Idx(chirp0Idx), chirp1Idx(chirp1Idx) {
    }

    static ConfigParameterPtr parse(ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const BpmCfg&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Config::Parameters
