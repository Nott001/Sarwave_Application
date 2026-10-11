#pragma once

#include <concepts>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "MMWave/Common/DotNetFormat.hpp"

namespace Sarwave::Core::MMWave::Config {

/// Writes a command line the way the device's CLI reads it: the command word, then each argument
/// separated by one space. Numbers use invariant culture, and floats are written in their
/// shortest form that reads back to the same value, so "30.00" in a source file comes out as
/// "30". A bool is written as 1 or 0, and an enum as its number, never its name.
struct ConfigText {
    template <class... Values>
    static std::string line(std::string_view name, const Values&... values) {
        std::string line(name);
        (append(line, values), ...);
        return line;
    }

    /// A line whose arguments are all of one kind, such as the twelve antenna indexes.
    template <std::ranges::input_range Range>
    static std::string list(std::string_view name, const Range& values) {
        std::string line(name);
        for (const auto& value : values) {
            append(line, value);
        }

        return line;
    }

    template <class T>
    static void append(std::string& line, const T& value) {
        line += ' ';
        line += format(value);
    }

   private:
    template <class T>
    static std::string format(const T& value) {
        if constexpr (std::same_as<T, bool>) {
            return value ? "1" : "0";
        } else if constexpr (std::is_enum_v<T>) {
            return std::to_string(std::to_underlying(value));
        } else if constexpr (std::same_as<T, float>) {
            return DotNet::formatSingle(value);
        } else if constexpr (std::same_as<T, double>) {
            return DotNet::formatDouble(value);
        } else if constexpr (std::integral<T>) {
            return std::to_string(value);
        } else {
            static_assert(std::convertible_to<const T&, std::string_view>,
                          "ConfigText writes bools, enums, numbers and text");
            return std::string(std::string_view(value));
        }
    }
};

}  // namespace Sarwave::Core::MMWave::Config
