#include "MMWave/Config/Parameters/ChirpCfg.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

ConfigParameterPtr ChirpCfg::parse(ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const ChirpCfg>(ChirpCfg{
        args.intValue("chirpStartIdx"), args.intValue("chirpEndIdx"), args.intValue("profileId"),
        args.floatValue("startFreqVar"), args.floatValue("freqSlopeVar"),
        args.floatValue("idleTimeVar"), args.floatValue("adcStartTimeVar"),
        args.enumValue<ChannelCfg::Tx>("txEnableMask")});
}

std::string ChirpCfg::toString() const {
    return ConfigText::line(Name, chirpStartIdx, chirpEndIdx, profileId, startFreqVar, freqSlopeVar,
                            idleTimeVar, adcStartTimeVar, txEnableMask);
}

}  // namespace Sarwave::Core::MMWave::Config::Parameters
