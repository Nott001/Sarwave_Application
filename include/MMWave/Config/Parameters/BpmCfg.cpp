#include "MMWave/Config/Parameters/BpmCfg.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

ConfigParameterPtr BpmCfg::parse(ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const BpmCfg>(
        BpmCfg{args.intValue("subFrameIdx"), args.boolValue("enabled"), args.intValue("chirp0Idx"),
               args.intValue("chirp1Idx")});
}

std::string BpmCfg::toString() const {
    return ConfigText::line(Name, subFrameIdx, enabled, chirp0Idx, chirp1Idx);
}

}  // namespace Sarwave::Core::MMWave::Config::Parameters
