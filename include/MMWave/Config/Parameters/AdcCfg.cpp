#include "MMWave/Config/Parameters/AdcCfg.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

ConfigParameterPtr AdcCfg::parse(ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const AdcCfg>(AdcCfg{args.enumValue<BitDepth>("numADCBits"),
                                                 args.enumValue<OutputFormat>("adcOutputFmt")});
}

std::string AdcCfg::toString() const {
    return ConfigText::line(Name, numAdcBits, adcOutputFmt);
}

const EnumDefinition<AdcCfg::BitDepth>& enumDefinition(AdcCfg::BitDepth) {
    static const EnumDefinition<AdcCfg::BitDepth> definition{
        false,
        {
            {AdcCfg::BitDepth::Bits12, "Bits12"},
            {AdcCfg::BitDepth::Bits14, "Bits14"},
            {AdcCfg::BitDepth::Bits16, "Bits16"},
        }};
    return definition;
}

const EnumDefinition<AdcCfg::OutputFormat>& enumDefinition(AdcCfg::OutputFormat) {
    static const EnumDefinition<AdcCfg::OutputFormat> definition{
        false,
        {
            {AdcCfg::OutputFormat::Real, "Real"},
            {AdcCfg::OutputFormat::Complex1x, "Complex1x"},
            {AdcCfg::OutputFormat::Complex2x, "Complex2x"},
        }};
    return definition;
}

}  // namespace Sarwave::Core::MMWave::Config::Parameters
