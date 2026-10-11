#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/ConfigEnum.hpp"
#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

/// `profileCfg`: The chirp template: where the frequency ramp starts, how steep it is and when it
/// is sampled. See SWRA553 for how these translate into range and velocity limits.
struct ProfileCfg final : ConfigRecord<ProfileCfg> {
    /// Corner frequencies of high-pass filter 1.
    enum class Hpf1Corner : int {
        Khz175 = 0,
        Khz235 = 1,
        Khz350 = 2,
        Khz700 = 3,
    };

    /// Corner frequencies of high-pass filter 2.
    enum class Hpf2Corner : int {
        Khz350 = 0,
        Khz700 = 1,
        /// 1.4 MHz.
        Mhz1_4 = 2,
        /// 2.8 MHz.
        Mhz2_8 = 3,
    };

    static constexpr std::string_view Name = "profileCfg";

    /// Profile identifier; people tracking supports only profile 0.
    int profileId;
    /// Frequency the ramp starts at, in GHz.
    float startFreq;
    /// Time between the end of one chirp and the start of the next, in microseconds.
    float idleTime;
    /// Time from ramp start until ADC sampling begins, in microseconds.
    float adcStartTime;
    /// Time from ramp start until the ramp stops, in microseconds. Must exceed ADC start time plus
    /// sampling time.
    float rampEndTime;
    /// Transmit power back-off code: one byte per transmitter (TX0 in bits 7:0, TX1 in 15:8, TX2 in
    /// 23:16), each in dB below maximum.
    int txOutPower;
    /// Transmit phase shifter code; always 0.
    int txPhaseShifter;
    /// Slope of the frequency ramp, in MHz per microsecond.
    float freqSlopeConst;
    /// Time from ramp start until the transmitter turns on, in microseconds.
    float txStartTime;
    /// ADC samples taken per chirp.
    int numAdcSamples;
    /// ADC sampling rate, in thousands of samples per second.
    int digOutSampleRate;
    /// Corner frequency of high-pass filter 1.
    Hpf1Corner hpfCornerFreq1;
    /// Corner frequency of high-pass filter 2.
    Hpf2Corner hpfCornerFreq2;
    /// Receive gain in dB, OR'ed with the RF gain target.
    int rxGain;

    ProfileCfg(int profileId, float startFreq, float idleTime, float adcStartTime,
               float rampEndTime, int txOutPower, int txPhaseShifter, float freqSlopeConst,
               float txStartTime, int numAdcSamples, int digOutSampleRate,
               Hpf1Corner hpfCornerFreq1, Hpf2Corner hpfCornerFreq2, int rxGain)
        : profileId(profileId),
          startFreq(startFreq),
          idleTime(idleTime),
          adcStartTime(adcStartTime),
          rampEndTime(rampEndTime),
          txOutPower(txOutPower),
          txPhaseShifter(txPhaseShifter),
          freqSlopeConst(freqSlopeConst),
          txStartTime(txStartTime),
          numAdcSamples(numAdcSamples),
          digOutSampleRate(digOutSampleRate),
          hpfCornerFreq1(hpfCornerFreq1),
          hpfCornerFreq2(hpfCornerFreq2),
          rxGain(rxGain) {
    }

    static ConfigParameterPtr parse(ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const ProfileCfg&) const = default;

    friend const EnumDefinition<Hpf1Corner>& enumDefinition(Hpf1Corner);

    friend const EnumDefinition<Hpf2Corner>& enumDefinition(Hpf2Corner);
};

}  // namespace Sarwave::Core::MMWave::Config::Parameters
