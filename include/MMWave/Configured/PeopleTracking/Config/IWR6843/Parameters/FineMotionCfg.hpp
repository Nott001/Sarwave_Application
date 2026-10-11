#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `fineMotionCfg`: Fine motion mode: detects people who are nearly still (sitting, sleeping) by
/// combining chirps across several frames. Optional; off when the line is absent. Enabling it
/// requires halving `frameCfg`'s loop count.
struct FineMotionCfg final : MMWave::Config::ConfigRecord<FineMotionCfg> {
    static constexpr std::string_view Name = "fineMotionCfg";

    /// Always -1; subframes are not supported.
    int subFrameIdx;
    /// Whether fine motion mode runs; the other fields are ignored when false.
    bool fineMotionProcEnabled;
    /// Observation window the multi-frame block should span, in seconds.
    float fineMotionObservationTime;
    /// The multi-frame block is processed once every this many frames.
    int fineMotionProcCycle;
    /// One-sided Doppler bin limit; points outside it are dropped in fine motion mode.
    int fineMotionDopplerThrIdx;

    FineMotionCfg(int subFrameIdx, bool fineMotionProcEnabled, float fineMotionObservationTime,
                  int fineMotionProcCycle, int fineMotionDopplerThrIdx)
        : subFrameIdx(subFrameIdx),
          fineMotionProcEnabled(fineMotionProcEnabled),
          fineMotionObservationTime(fineMotionObservationTime),
          fineMotionProcCycle(fineMotionProcCycle),
          fineMotionDopplerThrIdx(fineMotionDopplerThrIdx) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const FineMotionCfg&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
