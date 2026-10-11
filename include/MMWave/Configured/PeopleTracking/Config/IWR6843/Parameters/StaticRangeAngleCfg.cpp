#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/StaticRangeAngleCfg.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr StaticRangeAngleCfg::parse(
    MMWave::Config::ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const StaticRangeAngleCfg>(StaticRangeAngleCfg{
        args.intValue("subFrameIdx"), args.boolValue("staticProcEnabled"),
        args.intValue("staticAzimStepDeciFactor"), args.intValue("staticElevStepDeciFactor")});
}

std::string StaticRangeAngleCfg::toString() const {
    return MMWave::Config::ConfigText::line(Name, subFrameIdx, staticProcEnabled,
                                            staticAzimStepDeciFactor, staticElevStepDeciFactor);
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
