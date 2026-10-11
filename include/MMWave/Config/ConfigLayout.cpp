#include "MMWave/Config/ConfigLayout.hpp"

#include <utility>

namespace Sarwave::Core::MMWave::Config {

ConfigLayout::ConfigLayout(std::vector<ConfigParameterPtr> parameters)
    : m_parameters(std::move(parameters)) {
}

std::string ConfigLayout::toString(std::string_view newLine) const {
    std::string text;
    for (const ConfigParameterPtr& parameter : m_parameters) {
        text += parameter->toString();
        text += newLine;
    }

    return text;
}

}  // namespace Sarwave::Core::MMWave::Config
