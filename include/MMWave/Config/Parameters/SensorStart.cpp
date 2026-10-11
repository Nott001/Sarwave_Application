#include "MMWave/Config/Parameters/SensorStart.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

ConfigParameterPtr SensorStart::parse(ConfigArguments& args) {
    return std::make_shared<const SensorStart>(
        args.remaining() > 0 ? std::optional<int>(args.intValue("doReconfig")) : std::nullopt);
}

std::string SensorStart::toString() const {
    return doReconfig ? ConfigText::line(Name, *doReconfig) : std::string(Name);
}

}  // namespace Sarwave::Core::MMWave::Config::Parameters
