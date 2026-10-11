#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/StaticRACfarCfg.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr StaticRACfarCfg::parse(MMWave::Config::ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const StaticRACfarCfg>(StaticRACfarCfg{
        args.intValue("subFrameIdx"), args.intValue("cfarDiscardLeftRange"),
        args.intValue("cfarDiscardRightRange"), args.intValue("cfarDiscardLeftAngle"),
        args.intValue("cfarDiscardRightAngle"), args.intValue("refWinSizeRange"),
        args.intValue("refWinSizeAngle"), args.intValue("guardWinSizeRange"),
        args.intValue("guardWinSizeAngle"), args.floatValue("rangeThre"),
        args.floatValue("angleThre"), args.floatValue("sidelobeThre"),
        args.boolValue("enable2ndPass"), args.intValue("dynamicFlag")});
}

std::string StaticRACfarCfg::toString() const {
    return MMWave::Config::ConfigText::line(
        Name, subFrameIdx, cfarDiscardLeftRange, cfarDiscardRightRange, cfarDiscardLeftAngle,
        cfarDiscardRightAngle, refWinSizeRange, refWinSizeAngle, guardWinSizeRange,
        guardWinSizeAngle, rangeThre, angleThre, sidelobeThre, enable2ndPass, dynamicFlag);
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
