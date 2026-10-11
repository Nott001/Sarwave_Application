#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/DynamicRACfarCfg.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr DynamicRACfarCfg::parse(MMWave::Config::ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const DynamicRACfarCfg>(DynamicRACfarCfg{
        args.intValue("subFrameIdx"), args.intValue("cfarDiscardLeftRange"),
        args.intValue("cfarDiscardRightRange"), args.intValue("cfarDiscardLeftAngle"),
        args.intValue("cfarDiscardRightAngle"), args.intValue("refWinSizeRange"),
        args.intValue("refWinSizeAngle"), args.intValue("guardWinSizeRange"),
        args.intValue("guardWinSizeAngle"), args.floatValue("rangeThre"),
        args.floatValue("angleThre"), args.floatValue("sidelobeThre"),
        args.boolValue("enable2ndPass"), args.enumValue<DynamicMode>("dynamicFlag")});
}

std::string DynamicRACfarCfg::toString() const {
    return MMWave::Config::ConfigText::line(
        Name, subFrameIdx, cfarDiscardLeftRange, cfarDiscardRightRange, cfarDiscardLeftAngle,
        cfarDiscardRightAngle, refWinSizeRange, refWinSizeAngle, guardWinSizeRange,
        guardWinSizeAngle, rangeThre, angleThre, sidelobeThre, enable2ndPass, dynamicFlag);
}

const MMWave::Config::EnumDefinition<DynamicRACfarCfg::DynamicMode>& enumDefinition(
    DynamicRACfarCfg::DynamicMode) {
    static const MMWave::Config::EnumDefinition<DynamicRACfarCfg::DynamicMode> definition{
        false,
        {
            {DynamicRACfarCfg::DynamicMode::Disabled, "Disabled"},
            {DynamicRACfarCfg::DynamicMode::DisabledAlt, "DisabledAlt"},
            {DynamicRACfarCfg::DynamicMode::Range, "Range"},
            {DynamicRACfarCfg::DynamicMode::RangeAndAngle, "RangeAndAngle"},
        }};
    return definition;
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
