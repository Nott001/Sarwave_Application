#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config {

/// Maps command words to the parameter types that understand them. A layout's reader fills one
/// with the commands it supports and asks it to turn each line into a typed parameter.
class ConfigParameterRegistry {
   public:
    using Parser = ConfigParameterPtr (*)(ConfigArguments&);

    /// Registers `T` under its own `Name`, replacing any earlier entry.
    template <ParsableConfigParameter T>
    ConfigParameterRegistry& add() {
        m_parsers.insert_or_assign(std::string(T::Name),
                                   [](ConfigArguments& args) { return T::parse(args); });
        return *this;
    }

    [[nodiscard]] bool knows(std::string_view paramName) const;

    /// Builds the typed parameter for a line, or a RawParameter holding the line as written
    /// if no type is registered for `paramName`.
    ///
    /// Throws ConfigFormatException when the command is known but its arguments are missing,
    /// malformed, or too many.
    [[nodiscard]] ConfigParameterPtr parse(const std::string& paramName,
                                           std::vector<std::string> arguments) const;

   private:
    // Ordinal (case-sensitive) lookup, with string_view keys accepted as they are.
    struct Hash {
        using is_transparent = void;

        std::size_t operator()(std::string_view key) const {
            return std::hash<std::string_view>{}(key);
        }
    };

    std::unordered_map<std::string, Parser, Hash, std::equal_to<>> m_parsers;
};

}  // namespace Sarwave::Core::MMWave::Config
