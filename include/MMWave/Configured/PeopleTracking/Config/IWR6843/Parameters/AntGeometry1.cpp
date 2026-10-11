#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/AntGeometry1.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr AntGeometry1::parse(MMWave::Config::ConfigArguments& args) {
    return std::make_shared<const AntGeometry1>(args.remainingInts("virtAntIdx"));
}

std::string AntGeometry1::toString() const {
    return MMWave::Config::ConfigText::list(Name, elevationIndexes);
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
