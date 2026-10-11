#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/SensorPosition.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr SensorPosition::parse(MMWave::Config::ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const SensorPosition>(SensorPosition{
        args.floatValue("sensorHeight"), args.floatValue("azimTilt"), args.floatValue("elevTilt")});
}

std::string SensorPosition::toString() const {
    return MMWave::Config::ConfigText::line(Name, sensorHeight, azimTilt, elevTilt);
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
