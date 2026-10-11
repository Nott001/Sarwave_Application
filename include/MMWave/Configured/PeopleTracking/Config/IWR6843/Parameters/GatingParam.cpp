#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/GatingParam.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr GatingParam::parse(MMWave::Config::ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const GatingParam>(GatingParam{
        args.floatValue("gain"), args.floatValue("limitWidth"), args.floatValue("limitDepth"),
        args.floatValue("limitHeight"), args.floatValue("limitVelocity")});
}

std::string GatingParam::toString() const {
    return MMWave::Config::ConfigText::line(Name, gain, limitWidth, limitDepth, limitHeight,
                                            limitVelocity);
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
