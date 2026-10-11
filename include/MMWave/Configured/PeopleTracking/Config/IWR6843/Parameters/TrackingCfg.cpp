#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/TrackingCfg.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr TrackingCfg::parse(MMWave::Config::ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const TrackingCfg>(TrackingCfg{
        args.boolValue("enable"),
        args.intValue("initialConfigParams"),
        args.intValue("maxNumPoints"),
        args.intValue("maxNumTracks"),
        args.intValue("maxRadialVelocity"),
        args.intValue("radialVelocityResolution"),
        args.intValue("deltaT"),
        args.remaining() > 0 ? std::optional<bool>(args.boolValue("boresightFilteringEnable"))
                             : std::nullopt,
    });
}

std::string TrackingCfg::toString() const {
    using MMWave::Config::ConfigText;
    return boresightFilteringEnable
               ? ConfigText::line(Name, enable, initialConfigParams, maxNumPoints, maxNumTracks,
                                  maxRadialVelocity, radialVelocityResolution, deltaT,
                                  *boresightFilteringEnable)
               : ConfigText::line(Name, enable, initialConfigParams, maxNumPoints, maxNumTracks,
                                  maxRadialVelocity, radialVelocityResolution, deltaT);
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
