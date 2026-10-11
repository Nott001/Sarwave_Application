#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/Dynamic2DAngleCfg.hpp"

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

using MMWave::Config::ConfigArguments;
using MMWave::Config::ConfigParameterPtr;
using MMWave::Config::ConfigText;

ConfigParameterPtr Dynamic2DAngleCfg::parse(ConfigArguments& args) {
    switch (args.count()) {
        case Dynamic2DAngleElevationCfg::ArgumentCount:
            return Dynamic2DAngleElevationCfg::parseArguments(args);
        case Dynamic2DAngleZoomInCfg::ArgumentCount:
            return Dynamic2DAngleZoomInCfg::parseArguments(args);
        default:
            throw MMWave::Config::ConfigFormatException(
                std::string(Name) + ": expected " +
                std::to_string(Dynamic2DAngleElevationCfg::ArgumentCount) +
                " arguments (wall mount) or " +
                std::to_string(Dynamic2DAngleZoomInCfg::ArgumentCount) +
                " (ceiling mount), but the line has " + std::to_string(args.count()) + ".");
    }
}

std::shared_ptr<const Dynamic2DAngleElevationCfg> Dynamic2DAngleElevationCfg::parseArguments(
    ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const Dynamic2DAngleElevationCfg>(Dynamic2DAngleElevationCfg{
        args.intValue("subFrameIdx"),
        args.floatValue("elevSearchStep"),
        args.floatValue("angleDiagonalLoading"),
        args.intValue("maxNpeak2Search"),
        args.intValue("peakExpSamples"),
        args.boolValue("elevOnly"),
        args.floatValue("sidelobeThre"),
        args.floatValue("peakExpRelThre"),
        args.floatValue("peakExpSNRThre"),
    });
}

std::string Dynamic2DAngleElevationCfg::toString() const {
    return ConfigText::line(Name, subFrameIdx, elevSearchStep, angleDiagonalLoading,
                            maxNpeak2Search, peakExpSamples, elevOnly, sidelobeThre, peakExpRelThre,
                            peakExpSnrThre);
}

std::shared_ptr<const Dynamic2DAngleZoomInCfg> Dynamic2DAngleZoomInCfg::parseArguments(
    ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const Dynamic2DAngleZoomInCfg>(Dynamic2DAngleZoomInCfg{
        args.intValue("subFrameIdx"),
        args.intValue("zoominFactor"),
        args.intValue("zoominNn8bors"),
        args.intValue("peakExpSamples"),
        args.floatValue("peakExpRelThre"),
        args.floatValue("peakExpSNRThre"),
        args.enumValue<LocalMax>("localMaxCheck"),
    });
}

std::string Dynamic2DAngleZoomInCfg::toString() const {
    return ConfigText::line(Name, subFrameIdx, zoominFactor, zoominNn8bors, peakExpSamples,
                            peakExpRelThre, peakExpSnrThre, localMaxCheck);
}

const MMWave::Config::EnumDefinition<Dynamic2DAngleZoomInCfg::LocalMax>& enumDefinition(
    Dynamic2DAngleZoomInCfg::LocalMax) {
    static const MMWave::Config::EnumDefinition<Dynamic2DAngleZoomInCfg::LocalMax> definition{
        false,
        {
            {Dynamic2DAngleZoomInCfg::LocalMax::None, "None"},
            {Dynamic2DAngleZoomInCfg::LocalMax::Elevation, "Elevation"},
            {Dynamic2DAngleZoomInCfg::LocalMax::ElevationAndAzimuth, "ElevationAndAzimuth"},
        }};
    return definition;
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
