#include "MMWave/Config/ConfigParameterRegistry.hpp"

#include <memory>
#include <utility>

#include "MMWave/Config/RawParameter.hpp"

namespace Sarwave::Core::MMWave::Config {

bool ConfigParameterRegistry::knows(std::string_view paramName) const {
    return m_parsers.contains(paramName);
}

ConfigParameterPtr ConfigParameterRegistry::parse(const std::string& paramName,
                                                  std::vector<std::string> arguments) const {
    const auto entry = m_parsers.find(paramName);
    if (entry == m_parsers.end()) {
        return std::make_shared<const RawParameter>(paramName, std::move(arguments));
    }

    ConfigArguments args(paramName, arguments);
    ConfigParameterPtr parameter = entry->second(args);

    if (args.remaining() > 0) {
        throw ConfigFormatException(paramName + ": " + std::to_string(args.remaining()) +
                                    " more argument(s) than expected (the line has " +
                                    std::to_string(args.count()) + ").");
    }

    return parameter;
}

}  // namespace Sarwave::Core::MMWave::Config
