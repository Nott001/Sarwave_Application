#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/DynamicRangeAngleCfg.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr DynamicRangeAngleCfg::parse(
    MMWave::Config::ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const DynamicRangeAngleCfg>(
        DynamicRangeAngleCfg{args.intValue("subFrameIdx"), args.floatValue("angleSearchStep"),
                             args.floatValue("rangeAngleDiagonalLoading"),
                             args.enumValue<RangeAngleMethod>("rangeAngleEstMethod"),
                             args.enumValue<DopplerMethod>("dopplerEstMethod")});
}

std::string DynamicRangeAngleCfg::toString() const {
    return MMWave::Config::ConfigText::line(Name, subFrameIdx, angleSearchStep,
                                            rangeAngleDiagonalLoading, rangeAngleEstMethod,
                                            dopplerEstMethod);
}

const MMWave::Config::EnumDefinition<DynamicRangeAngleCfg::RangeAngleMethod>& enumDefinition(
    DynamicRangeAngleCfg::RangeAngleMethod) {
    static const MMWave::Config::EnumDefinition<DynamicRangeAngleCfg::RangeAngleMethod> definition{
        false,
        {
            {DynamicRangeAngleCfg::RangeAngleMethod::RangeAzimuthThenElevation,
             "RangeAzimuthThenElevation"},
            {DynamicRangeAngleCfg::RangeAngleMethod::RangeAzimuthElevationZoomIn,
             "RangeAzimuthElevationZoomIn"},
        }};
    return definition;
}

const MMWave::Config::EnumDefinition<DynamicRangeAngleCfg::DopplerMethod>& enumDefinition(
    DynamicRangeAngleCfg::DopplerMethod) {
    static const MMWave::Config::EnumDefinition<DynamicRangeAngleCfg::DopplerMethod> definition{
        false,
        {
            {DynamicRangeAngleCfg::DopplerMethod::SinglePeakSearch, "SinglePeakSearch"},
        }};
    return definition;
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
