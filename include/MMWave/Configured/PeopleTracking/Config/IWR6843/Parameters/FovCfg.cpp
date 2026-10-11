#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/FovCfg.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr FovCfg::parse(MMWave::Config::ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const FovCfg>(FovCfg{args.intValue("subFrameIdx"),
                                                 args.floatValue("azimuthFoV"),
                                                 args.floatValue("elevationFoV")});
}

std::string FovCfg::toString() const {
    return MMWave::Config::ConfigText::line(Name, subFrameIdx, azimuthFoV, elevationFoV);
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
