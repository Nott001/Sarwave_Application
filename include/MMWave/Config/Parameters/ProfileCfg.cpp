#include "MMWave/Config/Parameters/ProfileCfg.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

ConfigParameterPtr ProfileCfg::parse(ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const ProfileCfg>(
        ProfileCfg{args.intValue("profileId"), args.floatValue("startFreq"),
                   args.floatValue("idleTime"), args.floatValue("adcStartTime"),
                   args.floatValue("rampEndTime"), args.intValue("txOutPower"),
                   args.intValue("txPhaseShifter"), args.floatValue("freqSlopeConst"),
                   args.floatValue("txStartTime"), args.intValue("numAdcSamples"),
                   args.intValue("digOutSampleRate"), args.enumValue<Hpf1Corner>("hpfCornerFreq1"),
                   args.enumValue<Hpf2Corner>("hpfCornerFreq2"), args.intValue("rxGain")});
}

std::string ProfileCfg::toString() const {
    return ConfigText::line(Name, profileId, startFreq, idleTime, adcStartTime, rampEndTime,
                            txOutPower, txPhaseShifter, freqSlopeConst, txStartTime, numAdcSamples,
                            digOutSampleRate, hpfCornerFreq1, hpfCornerFreq2, rxGain);
}

const EnumDefinition<ProfileCfg::Hpf1Corner>& enumDefinition(ProfileCfg::Hpf1Corner) {
    static const EnumDefinition<ProfileCfg::Hpf1Corner> definition{
        false,
        {
            {ProfileCfg::Hpf1Corner::Khz175, "Khz175"},
            {ProfileCfg::Hpf1Corner::Khz235, "Khz235"},
            {ProfileCfg::Hpf1Corner::Khz350, "Khz350"},
            {ProfileCfg::Hpf1Corner::Khz700, "Khz700"},
        }};
    return definition;
}

const EnumDefinition<ProfileCfg::Hpf2Corner>& enumDefinition(ProfileCfg::Hpf2Corner) {
    static const EnumDefinition<ProfileCfg::Hpf2Corner> definition{
        false,
        {
            {ProfileCfg::Hpf2Corner::Khz350, "Khz350"},
            {ProfileCfg::Hpf2Corner::Khz700, "Khz700"},
            {ProfileCfg::Hpf2Corner::Mhz1_4, "Mhz1_4"},
            {ProfileCfg::Hpf2Corner::Mhz2_8, "Mhz2_8"},
        }};
    return definition;
}

}  // namespace Sarwave::Core::MMWave::Config::Parameters
