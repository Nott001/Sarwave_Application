#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/AllocationParam.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr AllocationParam::parse(MMWave::Config::ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const AllocationParam>(
        AllocationParam{args.floatValue("snrThre"), args.floatValue("snrThreObscured"),
                        args.floatValue("velocityThre"), args.floatValue("pointsThre"),
                        args.floatValue("maxDistanceThre"), args.floatValue("maxVelThre")});
}

std::string AllocationParam::toString() const {
    return MMWave::Config::ConfigText::line(Name, snrThre, snrThreObscured, velocityThre,
                                            pointsThre, maxDistanceThre, maxVelThre);
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
