#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/FineMotionCfg.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr FineMotionCfg::parse(MMWave::Config::ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const FineMotionCfg>(FineMotionCfg{
        args.intValue("subFrameIdx"), args.boolValue("fineMotionProcEnabled"),
        args.floatValue("fineMotionObservationTime"), args.intValue("fineMotionProcCycle"),
        args.intValue("fineMotionDopplerThrIdx")});
}

std::string FineMotionCfg::toString() const {
    return MMWave::Config::ConfigText::line(Name, subFrameIdx, fineMotionProcEnabled,
                                            fineMotionObservationTime, fineMotionProcCycle,
                                            fineMotionDopplerThrIdx);
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
