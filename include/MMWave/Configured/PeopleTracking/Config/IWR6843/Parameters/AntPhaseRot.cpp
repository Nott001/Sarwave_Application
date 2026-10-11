#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/AntPhaseRot.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr AntPhaseRot::parse(MMWave::Config::ConfigArguments& args) {
    return std::make_shared<const AntPhaseRot>(args.remainingEnums<Rotation>("virtAntIdx"));
}

std::string AntPhaseRot::toString() const {
    return MMWave::Config::ConfigText::list(Name, rotations);
}

const MMWave::Config::EnumDefinition<AntPhaseRot::Rotation>& enumDefinition(AntPhaseRot::Rotation) {
    static const MMWave::Config::EnumDefinition<AntPhaseRot::Rotation> definition{
        false,
        {
            {AntPhaseRot::Rotation::Inverted, "Inverted"},
            {AntPhaseRot::Rotation::None, "None"},
        }};
    return definition;
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
