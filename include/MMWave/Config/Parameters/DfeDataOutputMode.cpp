#include "MMWave/Config/Parameters/DfeDataOutputMode.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

ConfigParameterPtr DfeDataOutputMode::parse(ConfigArguments& args) {
    return std::make_shared<const DfeDataOutputMode>(
        DfeDataOutputMode{args.enumValue<Mode>("modeType")});
}

std::string DfeDataOutputMode::toString() const {
    return ConfigText::line(Name, modeType);
}

const EnumDefinition<DfeDataOutputMode::Mode>& enumDefinition(DfeDataOutputMode::Mode) {
    static const EnumDefinition<DfeDataOutputMode::Mode> definition{
        false,
        {
            {DfeDataOutputMode::Mode::FrameBased, "FrameBased"},
            {DfeDataOutputMode::Mode::Continuous, "Continuous"},
            {DfeDataOutputMode::Mode::AdvancedFrame, "AdvancedFrame"},
        }};
    return definition;
}

}  // namespace Sarwave::Core::MMWave::Config::Parameters
