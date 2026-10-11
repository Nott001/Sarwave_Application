#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "MMWave/Config/ConfigEnum.hpp"
#include "MMWave/Config/ConfigFormatException.hpp"

namespace Sarwave::Core::MMWave::Config {

/// The arguments of one command, handed out one at a time in file order. Every read names the
/// field it is for, so a bad or missing value is reported as e.g. "profileCfg: <idleTime> ...".
///
/// Holds a view of the argument list, which must outlive it (it lives for one parse call).
class ConfigArguments {
   public:
    ConfigArguments(std::string paramName, std::span<const std::string> values)
        : m_paramName(std::move(paramName)), m_values(values) {
    }

    [[nodiscard]] const std::string& paramName() const {
        return m_paramName;
    }

    /// How many arguments the line has in total.
    [[nodiscard]] int count() const {
        return static_cast<int>(m_values.size());
    }

    /// How many have not been read yet.
    [[nodiscard]] int remaining() const {
        return static_cast<int>(m_values.size() - m_next);
    }

    /// A whole number. Also accepts one written with decimals or an exponent ("2950.00",
    /// "1e3") as long as it has no fractional part.
    int intValue(std::string_view field);

    float floatValue(std::string_view field);

    /// A flag written as 0 (false) or 1 (true).
    bool boolValue(std::string_view field);

    /// A number with a documented meaning per value. Anything `TEnum` doesn't define is
    /// rejected; for a [Flags] enum, any combination of its defined bits is accepted.
    template <ConfigEnum TEnum>
    TEnum enumValue(std::string_view field) {
        const int raw = intValue(field);
        const auto value = static_cast<TEnum>(raw);
        const EnumDefinition<TEnum>& definition = enumDefinition(value);

        bool valid = false;
        if (definition.isFlags()) {
            int allBits = 0;
            for (const EnumMember<TEnum>& defined : definition.members()) {
                allBits |= static_cast<int>(defined.value);
            }

            valid = (raw & ~allBits) == 0;
        } else {
            valid = definition.isDefined(value);
        }

        if (valid) return value;

        std::vector<std::pair<int, std::string_view>> allowed;
        for (const EnumMember<TEnum>& defined : definition.members()) {
            allowed.emplace_back(static_cast<int>(defined.value), defined.name);
        }
        throwUndocumented(field, raw, definition.isFlags(), allowed);
    }

    /// Every argument that is left, each as a `TEnum`.
    template <ConfigEnum TEnum>
    std::vector<TEnum> remainingEnums(std::string_view field) {
        std::vector<TEnum> values(static_cast<std::size_t>(remaining()));
        for (std::size_t i = 0; i < values.size(); i++) {
            values[i] = enumValue<TEnum>(std::string(field) + " " + std::to_string(i));
        }

        return values;
    }

    /// Every argument that is left, as whole numbers.
    std::vector<int> remainingInts(std::string_view field);

   private:
    const std::string& next(std::string_view field);

    [[noreturn]] void throwUndocumented(
        std::string_view field, int raw, bool isFlags,
        std::span<const std::pair<int, std::string_view>> allowed) const;

    std::string m_paramName;
    std::span<const std::string> m_values;
    std::size_t m_next = 0;
};

}  // namespace Sarwave::Core::MMWave::Config
