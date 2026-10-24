#pragma once

// What the check tools share: .NET's number formats and the live app's session folder, without
// Qt.

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <string>

#include "MMWave/Common/DotNetFormat.hpp"

namespace SarwaveTools {

/// value.ToString("+0.00;-0.00") and the like: `decimals` places, always signed. Zero takes the
/// first section, so it prints as +0.00, and −0 as -0.00.
inline std::string signedFixed(float value, int decimals) {
    const bool negative = value < 0 || (value == 0 && std::signbit(value));
    return (negative ? "-" : "+") +
           Sarwave::Core::MMWave::DotNet::formatFixed(std::abs(value), decimals, decimals);
}

/// {text,width}: right-aligned in `width` characters (negative: left-aligned).
inline std::string align(std::string text, int width) {
    const std::size_t w = static_cast<std::size_t>(std::abs(width));
    if (text.size() >= w) return text;
    if (width > 0) return std::string(w - text.size(), ' ') + text;
    return text + std::string(w - text.size(), ' ');
}

/// Environment.SpecialFolder.LocalApplicationData/Sarwave, where the live app records.
inline std::filesystem::path sessionFolder() {
#ifdef _WIN32
    const char* local = std::getenv("LOCALAPPDATA");
    return std::filesystem::path(local ? local : "") / "Sarwave";
#else
    if (const char* xdg = std::getenv("XDG_DATA_HOME"); xdg && xdg[0] == '/')
        return std::filesystem::path(xdg) / "Sarwave";
    const char* home = std::getenv("HOME");
    return std::filesystem::path(home ? home : "") / ".local" / "share" / "Sarwave";
#endif
}

}  // namespace SarwaveTools
