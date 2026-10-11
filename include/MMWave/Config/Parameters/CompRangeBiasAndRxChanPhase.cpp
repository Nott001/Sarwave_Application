#include "MMWave/Config/Parameters/CompRangeBiasAndRxChanPhase.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Config::Parameters {

ConfigParameterPtr CompRangeBiasAndRxChanPhase::parse(ConfigArguments& args) {
    const float rangeBias = args.floatValue("rangeBias");
    if (args.remaining() % 2 != 0) {
        throw ConfigFormatException(
            std::string(Name) + ": the phase factors come in <Re> <Im> pairs, but " +
            std::to_string(args.remaining()) + " value(s) follow <rangeBias>.");
    }

    std::vector<PhaseFactor> phases(static_cast<std::size_t>(args.remaining() / 2));
    for (std::size_t i = 0; i < phases.size(); i++) {
        const std::string index = std::to_string(i);
        phases[i] = PhaseFactor{args.floatValue("Re " + index), args.floatValue("Im " + index)};
    }

    return std::make_shared<const CompRangeBiasAndRxChanPhase>(rangeBias, std::move(phases));
}

std::string CompRangeBiasAndRxChanPhase::toString() const {
    std::string line(Name);
    ConfigText::append(line, rangeBias);
    for (const auto& [re, im] : rxChanPhase) {
        ConfigText::append(line, re);
        ConfigText::append(line, im);
    }

    return line;
}

}  // namespace Sarwave::Core::MMWave::Config::Parameters
