#include "MMWave/Config/Parameters/AdcbufCfg.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

ConfigParameterPtr AdcbufCfg::parse(ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const AdcbufCfg>(AdcbufCfg{
        args.intValue("subFrameIdx"), args.enumValue<OutputFormat>("adcOutputFmt"),
        args.enumValue<IqOrder>("sampleSwap"), args.enumValue<ChannelLayout>("chanInterleave"),
        args.intValue("chirpThreshold")});
}

std::string AdcbufCfg::toString() const {
    return ConfigText::line(Name, subFrameIdx, adcOutputFmt, sampleSwap, chanInterleave,
                            chirpThreshold);
}

const EnumDefinition<AdcbufCfg::OutputFormat>& enumDefinition(AdcbufCfg::OutputFormat) {
    static const EnumDefinition<AdcbufCfg::OutputFormat> definition{
        false,
        {
            {AdcbufCfg::OutputFormat::Complex, "Complex"},
            {AdcbufCfg::OutputFormat::Real, "Real"},
        }};
    return definition;
}

const EnumDefinition<AdcbufCfg::IqOrder>& enumDefinition(AdcbufCfg::IqOrder) {
    static const EnumDefinition<AdcbufCfg::IqOrder> definition{
        false,
        {
            {AdcbufCfg::IqOrder::ILsbQMsb, "ILsbQMsb"},
            {AdcbufCfg::IqOrder::QLsbIMsb, "QLsbIMsb"},
        }};
    return definition;
}

const EnumDefinition<AdcbufCfg::ChannelLayout>& enumDefinition(AdcbufCfg::ChannelLayout) {
    static const EnumDefinition<AdcbufCfg::ChannelLayout> definition{
        false,
        {
            {AdcbufCfg::ChannelLayout::Interleaved, "Interleaved"},
            {AdcbufCfg::ChannelLayout::NonInterleaved, "NonInterleaved"},
        }};
    return definition;
}

}  // namespace Sarwave::Core::MMWave::Config::Parameters
