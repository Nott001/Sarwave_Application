#include "MMWave/Config/Parameters/ChannelCfg.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

ConfigParameterPtr ChannelCfg::parse(ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const ChannelCfg>(ChannelCfg{args.enumValue<Rx>("rxChannelEn"),
                                                         args.enumValue<Tx>("txChannelEn"),
                                                         args.intValue("cascading")});
}

std::string ChannelCfg::toString() const {
    return ConfigText::line(Name, rxChannelEn, txChannelEn, cascading);
}

const EnumDefinition<ChannelCfg::Rx>& enumDefinition(ChannelCfg::Rx) {
    static const EnumDefinition<ChannelCfg::Rx> definition{true,
                                                           {
                                                               {ChannelCfg::Rx::None, "None"},
                                                               {ChannelCfg::Rx::Rx0, "Rx0"},
                                                               {ChannelCfg::Rx::Rx1, "Rx1"},
                                                               {ChannelCfg::Rx::Rx2, "Rx2"},
                                                               {ChannelCfg::Rx::Rx3, "Rx3"},
                                                           }};
    return definition;
}

const EnumDefinition<ChannelCfg::Tx>& enumDefinition(ChannelCfg::Tx) {
    static const EnumDefinition<ChannelCfg::Tx> definition{true,
                                                           {
                                                               {ChannelCfg::Tx::None, "None"},
                                                               {ChannelCfg::Tx::Tx0, "Tx0"},
                                                               {ChannelCfg::Tx::Tx1, "Tx1"},
                                                               {ChannelCfg::Tx::Tx2, "Tx2"},
                                                           }};
    return definition;
}

}  // namespace Sarwave::Core::MMWave::Config::Parameters
