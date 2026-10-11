#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/AntGeometry0.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr AntGeometry0::parse(MMWave::Config::ConfigArguments& args) {
    return std::make_shared<const AntGeometry0>(args.remainingInts("virtAntIdx"));
}

std::string AntGeometry0::toString() const {
    return MMWave::Config::ConfigText::list(Name, azimuthIndexes);
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
