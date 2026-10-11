#include "MMWave/Config/ConfigArguments.hpp"

#include <cmath>
#include <cstdint>
#include <limits>

#include "MMWave/Common/DotNetFormat.hpp"

namespace Sarwave::Core::MMWave::Config {
namespace {

std::string quoted(std::string_view field) {
    return "<" + std::string(field) + ">";
}

/// (int)value for a float with no fractional part, saturating like .NET's float to int
/// conversion does since .NET 9 (2147483648f becomes int.MaxValue).
int saturatingToInt(float value) {
    if (value >= 2147483648.0f) return std::numeric_limits<int>::max();
    if (value <= -2147483648.0f) return std::numeric_limits<int>::min();
    return static_cast<int>(value);
}

}  // namespace

int ConfigArguments::intValue(std::string_view field) {
    const std::string& text = next(field);
    std::int32_t value = 0;
    if (DotNet::tryParseInt32(text, value)) return value;

    // The shipped .cfg files write some integer fields with decimals ("2950.00").
    float asFloat = 0;
    if (DotNet::tryParseSingle(text, asFloat) && asFloat == std::trunc(asFloat) &&
        std::abs(asFloat) <= static_cast<float>(std::numeric_limits<int>::max())) {
        return saturatingToInt(asFloat);
    }

    throw ConfigFormatException(m_paramName + ": " + quoted(field) +
                                " should be a whole number, but is '" + text + "'.");
}

float ConfigArguments::floatValue(std::string_view field) {
    const std::string& text = next(field);
    float value = 0;
    if (DotNet::tryParseSingle(text, value)) return value;

    throw ConfigFormatException(m_paramName + ": " + quoted(field) +
                                " should be a number, but is '" + text + "'.");
}

bool ConfigArguments::boolValue(std::string_view field) {
    const std::string& text = next(field);
    if (text == "0") return false;
    if (text == "1") return true;

    throw ConfigFormatException(m_paramName + ": " + quoted(field) + " should be 0 or 1, but is '" +
                                text + "'.");
}

std::vector<int> ConfigArguments::remainingInts(std::string_view field) {
    std::vector<int> values(static_cast<std::size_t>(remaining()));
    for (std::size_t i = 0; i < values.size(); i++) {
        values[i] = intValue(std::string(field) + " " + std::to_string(i));
    }

    return values;
}

const std::string& ConfigArguments::next(std::string_view field) {
    if (m_next >= m_values.size()) {
        throw ConfigFormatException(m_paramName + ": " + quoted(field) +
                                    " is missing (the line has " + std::to_string(m_values.size()) +
                                    " argument(s)).");
    }

    return m_values[m_next++];
}

void ConfigArguments::throwUndocumented(
    std::string_view field, int raw, bool isFlags,
    std::span<const std::pair<int, std::string_view>> allowed) const {
    std::string list;
    for (const auto& [value, name] : allowed) {
        if (!list.empty()) list += ", ";
        list += std::to_string(value) + " (" + std::string(name) + ")";
    }

    throw ConfigFormatException(m_paramName + ": " + quoted(field) + " is " + std::to_string(raw) +
                                ", which is not a documented value. Expected " +
                                (isFlags ? "a combination of" : "one of") + ": " + list + ".");
}

}  // namespace Sarwave::Core::MMWave::Config
