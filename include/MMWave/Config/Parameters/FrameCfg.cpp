#include "MMWave/Config/Parameters/FrameCfg.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

ConfigParameterPtr FrameCfg::parse(ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const FrameCfg>(FrameCfg{
        args.intValue("chirpStartIdx"), args.intValue("chirpEndIdx"), args.intValue("numLoops"),
        args.intValue("numFrames"), args.floatValue("framePeriodicity"),
        args.enumValue<Trigger>("triggerSelect"), args.floatValue("frameTriggerDelay")});
}

std::string FrameCfg::toString() const {
    return ConfigText::line(Name, chirpStartIdx, chirpEndIdx, numLoops, numFrames, framePeriodicity,
                            triggerSelect, frameTriggerDelay);
}

const EnumDefinition<FrameCfg::Trigger>& enumDefinition(FrameCfg::Trigger) {
    static const EnumDefinition<FrameCfg::Trigger> definition{
        false,
        {
            {FrameCfg::Trigger::Software, "Software"},
            {FrameCfg::Trigger::Hardware, "Hardware"},
        }};
    return definition;
}

}  // namespace Sarwave::Core::MMWave::Config::Parameters
