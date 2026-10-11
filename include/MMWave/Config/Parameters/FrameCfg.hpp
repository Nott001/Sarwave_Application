#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/ConfigEnum.hpp"
#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

/// `frameCfg`: How chirps repeat within a frame and how often frames occur.
struct FrameCfg final : ConfigRecord<FrameCfg> {
    /// What can start a frame.
    enum class Trigger : int {
        Software = 1,
        Hardware = 2,
    };

    static constexpr std::string_view Name = "frameCfg";

    /// First chirp index of the sequence (0-511).
    int chirpStartIdx;
    /// Last chirp index of the sequence.
    int chirpEndIdx;
    /// Times the chirp sequence repeats within one frame (1-255).
    int numLoops;
    /// Frames to transmit; 0 = run until stopped.
    int numFrames;
    /// Time from the start of one frame to the next, in milliseconds.
    float framePeriodicity;
    /// What starts each frame. Only the software trigger is supported.
    Trigger triggerSelect;
    /// Delay after the trigger before the frame starts, in milliseconds.
    float frameTriggerDelay;

    FrameCfg(int chirpStartIdx, int chirpEndIdx, int numLoops, int numFrames,
             float framePeriodicity, Trigger triggerSelect, float frameTriggerDelay)
        : chirpStartIdx(chirpStartIdx),
          chirpEndIdx(chirpEndIdx),
          numLoops(numLoops),
          numFrames(numFrames),
          framePeriodicity(framePeriodicity),
          triggerSelect(triggerSelect),
          frameTriggerDelay(frameTriggerDelay) {
    }

    static ConfigParameterPtr parse(ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const FrameCfg&) const = default;

    friend const EnumDefinition<Trigger>& enumDefinition(Trigger);
};

}  // namespace Sarwave::Core::MMWave::Config::Parameters
