#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"
#include "MMWave/Config/Parameters/ChannelCfg.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

/// `chirpCfg`: Assigns a range of chirp indexes to a profile and to the transmitter(s) that fire
/// them. Appears once per distinct chirp.
struct ChirpCfg final : ConfigRecord<ChirpCfg> {
    static constexpr std::string_view Name = "chirpCfg";

    /// First chirp index this line defines.
    int chirpStartIdx;
    /// Last chirp index this line defines.
    int chirpEndIdx;
    /// The ProfileCfg these chirps use.
    int profileId;
    /// Start frequency variation from the profile, in Hz.
    float startFreqVar;
    /// Slope variation from the profile, in kHz per microsecond.
    float freqSlopeVar;
    /// Idle time variation from the profile, in microseconds.
    float idleTimeVar;
    /// ADC start time variation from the profile, in microseconds.
    float adcStartTimeVar;
    /// Transmitters that fire these chirps: a single one for TDM, a pair for BPM.
    ChannelCfg::Tx txEnableMask;

    ChirpCfg(int chirpStartIdx, int chirpEndIdx, int profileId, float startFreqVar,
             float freqSlopeVar, float idleTimeVar, float adcStartTimeVar,
             ChannelCfg::Tx txEnableMask)
        : chirpStartIdx(chirpStartIdx),
          chirpEndIdx(chirpEndIdx),
          profileId(profileId),
          startFreqVar(startFreqVar),
          freqSlopeVar(freqSlopeVar),
          idleTimeVar(idleTimeVar),
          adcStartTimeVar(adcStartTimeVar),
          txEnableMask(txEnableMask) {
    }

    static ConfigParameterPtr parse(ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const ChirpCfg&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Config::Parameters
