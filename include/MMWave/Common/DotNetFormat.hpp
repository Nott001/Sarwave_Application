#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "MMWave/Common/TimeSpan.hpp"

/// Text formatting that reproduces what .NET prints, so console output, .cfg files and on-screen
/// figures come out character for character the same as the C# application's.
namespace Sarwave::Core::MMWave::DotNet {

/// float.ToString() with the invariant culture: the shortest text that reads back to the same
/// float, switching to "E+XX" notation the way .NET does ("1E+10", "1E-05", "-0", "NaN",
/// "Infinity").
std::string formatSingle(float value);

/// double.ToString() with the invariant culture (shortest round-trip, .NET layout).
std::string formatDouble(double value);

/// A TimeSpan through the custom format @"mm\:ss\.ffffff": minutes and seconds of the
/// interval (two digits each) and the fraction of a second truncated to six digits.
std::string formatMinutesSecondsMicros(TimeSpan span);

/// Value formatted with .NET's custom numeric format "0.##", "0.00", "0.000", "0.0", "0.#",
/// "0": `minDecimals` mandatory decimals, up to `maxDecimals` in total, rounded half away
/// from zero. Used for the GUI's StringFormat bindings and radar labels.
std::string formatFixed(double value, int minDecimals, int maxDecimals);

/// The same for a float: .NET formats a float from its 7 most significant digits (a double from
/// 15) before rounding, so e.g. 0.12499999f with "0.00" gives "0.13" where the double gives "0.12".
std::string formatFixed(float value, int minDecimals, int maxDecimals);

/// int.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture): optional leading and
/// trailing white space, an optional sign, then decimal digits that fit a 32-bit int.
bool tryParseInt32(std::string_view text, std::int32_t& value);

/// float.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture): white space, sign,
/// digits with an optional '.', an optional exponent, or "Infinity"/"NaN" (case-insensitive).
bool tryParseSingle(std::string_view text, float& value);

}  // namespace Sarwave::Core::MMWave::DotNet
