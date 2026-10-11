#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/MaxAcceleration.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr MaxAcceleration::parse(MMWave::Config::ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const MaxAcceleration>(MaxAcceleration{
        args.floatValue("maxAccelX"), args.floatValue("maxAccelY"), args.floatValue("maxAccelZ")});
}

std::string MaxAcceleration::toString() const {
    return MMWave::Config::ConfigText::line(Name, x, y, z);
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
