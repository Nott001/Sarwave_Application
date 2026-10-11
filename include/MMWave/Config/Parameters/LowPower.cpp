#include "MMWave/Config/Parameters/LowPower.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

ConfigParameterPtr LowPower::parse(ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const LowPower>(
        LowPower{args.intValue("dontCare"), args.enumValue<Mode>("adcMode")});
}

std::string LowPower::toString() const {
    return ConfigText::line(Name, dontCare, adcMode);
}

const EnumDefinition<LowPower::Mode>& enumDefinition(LowPower::Mode) {
    static const EnumDefinition<LowPower::Mode> definition{
        false,
        {
            {LowPower::Mode::Regular, "Regular"},
            {LowPower::Mode::LowPower, "LowPower"},
        }};
    return definition;
}

}  // namespace Sarwave::Core::MMWave::Config::Parameters
