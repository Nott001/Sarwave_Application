#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/StateParam.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr StateParam::parse(MMWave::Config::ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const StateParam>(
        StateParam{args.intValue("det2actThre"), args.intValue("det2freeThre"),
                   args.intValue("active2freeThre"), args.intValue("static2freeThre"),
                   args.intValue("exit2freeThre"), args.intValue("sleep2freeThre")});
}

std::string StateParam::toString() const {
    return MMWave::Config::ConfigText::line(Name, det2ActThre, det2FreeThre, active2FreeThre,
                                            static2FreeThre, exit2FreeThre, sleep2FreeThre);
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
