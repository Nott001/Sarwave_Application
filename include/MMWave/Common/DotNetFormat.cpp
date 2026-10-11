#include "MMWave/Common/DotNetFormat.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <system_error>

namespace Sarwave::Core::MMWave::DotNet {
namespace {

/// Lays out shortest round-trip digits the way .NET's "G"/"R" formatting does
/// (Number.Formatting.cs, FormatGeneral): fixed notation unless the decimal point would land
/// more than `precision` digits out or more than 3 zeros in, then d.dddE+XX.
template <class T>
std::string formatShortest(T value, int precision) {
    if (std::isnan(value)) return "NaN";
    if (std::isinf(value)) return value > 0 ? "Infinity" : "-Infinity";

    const bool negative = std::signbit(value);
    if (value == 0) return negative ? "-0" : "0";

    // Shortest digits in scientific form: "d.dddde+XX" / "de-XX".
    std::array<char, 64> buf{};
    auto [end, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), std::abs(value),
                                   std::chars_format::scientific);
    std::string_view sci(buf.data(), static_cast<std::size_t>(end - buf.data()));

    const auto ePos = sci.find('e');
    std::string digits;
    for (char c : sci.substr(0, ePos)) {
        if (c != '.') digits += c;
    }
    int exponent = 0;
    std::from_chars(sci.data() + ePos + 1 + (sci[ePos + 1] == '+' ? 1 : 0), sci.data() + sci.size(),
                    exponent);

    const int digitCount = static_cast<int>(digits.size());
    const int scale = exponent + 1;  // position of the decimal point relative to the digits
    const int maxDigits = std::max(digitCount, precision);

    std::string out;
    if (negative) out += '-';

    if (scale > maxDigits || scale < -3) {
        out += digits[0];
        if (digitCount > 1) {
            out += '.';
            out.append(digits, 1);
        }
        const int shown = scale - 1;
        out += shown < 0 ? "E-" : "E+";
        const int absExp = std::abs(shown);
        if (absExp < 10) out += '0';
        out += std::to_string(absExp);
        return out;
    }

    if (scale <= 0) {
        out += "0.";
        out.append(static_cast<std::size_t>(-scale), '0');
        out += digits;
    } else if (scale >= digitCount) {
        out += digits;
        out.append(static_cast<std::size_t>(scale - digitCount), '0');
    } else {
        out.append(digits, 0, static_cast<std::size_t>(scale));
        out += '.';
        out.append(digits, static_cast<std::size_t>(scale));
    }
    return out;
}

bool isSpace(char c) {
    // char.IsWhiteSpace for the ASCII range that NumberStyles.Allow*White accepts.
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

std::string_view trim(std::string_view text) {
    while (!text.empty() && isSpace(text.front())) text.remove_prefix(1);
    while (!text.empty() && isSpace(text.back())) text.remove_suffix(1);
    return text;
}

/// .NET's number parsing accepts NUL characters after the number and its trailing white space
/// (Number.Parsing's TrailingZeros), so "1\0" reads as 1.
std::string_view trimTrailingNuls(std::string_view text) {
    while (!text.empty() && text.back() == '\0') text.remove_suffix(1);
    return text;
}

bool equalsIgnoreCase(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? char(c - 'A' + 'a') : c; };
        if (lower(a[i]) != lower(b[i])) return false;
    }
    return true;
}

}  // namespace

std::string formatSingle(float value) {
    return formatShortest(value, 9);  // SinglePrecision
}

std::string formatDouble(double value) {
    return formatShortest(value, 15);  // DoublePrecision
}

std::string formatMinutesSecondsMicros(TimeSpan span) {
    const std::int64_t ticks = span.count();
    const auto minutes = std::llabs((ticks / TimeSpans::TicksPerMinute) % 60);
    const auto seconds = std::llabs((ticks / TimeSpans::TicksPerSecond) % 60);
    const auto micros = std::llabs(ticks % TimeSpans::TicksPerSecond) / 10;

    std::array<char, 32> buf{};
    std::snprintf(buf.data(), buf.size(), "%02lld:%02lld.%06lld", static_cast<long long>(minutes),
                  static_cast<long long>(seconds), static_cast<long long>(micros));
    return buf.data();
}

std::string formatFixedDigits(double value, int minDecimals, int maxDecimals,
                              int significantDigits);

std::string formatFixed(double value, int minDecimals, int maxDecimals) {
    return formatFixedDigits(value, minDecimals, maxDecimals, 15);
}

std::string formatFixed(float value, int minDecimals, int maxDecimals) {
    return formatFixedDigits(static_cast<double>(value), minDecimals, maxDecimals, 7);
}

std::string formatFixedDigits(double value, int minDecimals, int maxDecimals,
                              int significantDigits) {
    if (std::isnan(value)) return "NaN";
    if (std::isinf(value)) return value > 0 ? "Infinity" : "-Infinity";

    // .NET first takes 15 (double) or 7 (float) significant digits, then rounds those half away
    // from zero at the last decimal the format allows.
    std::array<char, 64> buf{};
    std::snprintf(buf.data(), buf.size(), "%.*e", significantDigits - 1, std::abs(value));
    std::string_view sci(buf.data());
    const auto ePos = sci.find('e');
    std::string digits;
    for (char c : sci.substr(0, ePos)) {
        if (c != '.') digits += c;
    }
    const int exponent = std::atoi(sci.data() + ePos + 1);

    // Integer and fraction digits relative to the decimal point: value = 0.digits * 10^scale.
    int scale = exponent + 1;
    if (std::abs(value) == 0.0) {
        digits = "0";
        scale = 1;
    }

    // Keep scale + maxDecimals digits and round on the next one.
    const int keep = scale + maxDecimals;
    std::string kept;
    bool roundUp = false;
    if (keep < 0) {
        kept.clear();
    } else {
        kept =
            digits.substr(0, std::min<std::size_t>(digits.size(), static_cast<std::size_t>(keep)));
        if (static_cast<std::size_t>(keep) < digits.size())
            roundUp = digits[static_cast<std::size_t>(keep)] >= '5';
        if (kept.size() < static_cast<std::size_t>(keep))
            kept.append(static_cast<std::size_t>(keep) - kept.size(), '0');
    }
    if (roundUp) {
        int i = static_cast<int>(kept.size()) - 1;
        while (i >= 0 && kept[static_cast<std::size_t>(i)] == '9') {
            kept[static_cast<std::size_t>(i)] = '0';
            --i;
        }
        if (i >= 0) {
            ++kept[static_cast<std::size_t>(i)];
        } else {
            kept.insert(kept.begin(), '1');
            ++scale;
        }
    }

    // Split into integer / fraction parts.
    std::string integerPart;
    std::string fraction;
    if (scale > 0) {
        integerPart =
            kept.substr(0, std::min<std::size_t>(kept.size(), static_cast<std::size_t>(scale)));
        if (integerPart.size() < static_cast<std::size_t>(scale))
            integerPart.append(static_cast<std::size_t>(scale) - integerPart.size(), '0');
        if (kept.size() > static_cast<std::size_t>(scale))
            fraction = kept.substr(static_cast<std::size_t>(scale));
    } else {
        fraction = std::string(static_cast<std::size_t>(std::min(-scale, maxDecimals)), '0') + kept;
    }
    integerPart.erase(0, std::min(integerPart.find_first_not_of('0'), integerPart.size()));
    if (integerPart.empty()) integerPart = "0";
    if (fraction.size() > static_cast<std::size_t>(maxDecimals))
        fraction.resize(static_cast<std::size_t>(maxDecimals));
    while (fraction.size() > static_cast<std::size_t>(minDecimals) && fraction.back() == '0')
        fraction.pop_back();
    if (fraction.size() < static_cast<std::size_t>(minDecimals))
        fraction.append(static_cast<std::size_t>(minDecimals) - fraction.size(), '0');

    // .NET keeps the sign even when the value rounds to zero (-0.001 -> "-0.00", -0.0 -> "-0.00").
    std::string out;
    if (std::signbit(value)) out += '-';
    out += integerPart;
    if (!fraction.empty()) {
        out += '.';
        out += fraction;
    }
    return out;
}

bool tryParseInt32(std::string_view text, std::int32_t& value) {
    text = trim(trimTrailingNuls(text));
    bool negative = false;
    if (!text.empty() && (text.front() == '+' || text.front() == '-')) {
        negative = text.front() == '-';
        text.remove_prefix(1);
    }
    if (text.empty()) return false;

    std::int64_t magnitude = 0;
    for (char c : text) {
        if (c < '0' || c > '9') return false;
        magnitude = magnitude * 10 + (c - '0');
        if (magnitude > static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::max()) + 1)
            return false;
    }
    const std::int64_t signedValue = negative ? -magnitude : magnitude;
    if (signedValue > std::numeric_limits<std::int32_t>::max() ||
        signedValue < std::numeric_limits<std::int32_t>::min()) {
        return false;
    }
    value = static_cast<std::int32_t>(signedValue);
    return true;
}

bool tryParseSingle(std::string_view text, float& value) {
    // Trailing NULs are allowed after a number, not after the "Infinity"/"NaN" symbols.
    const std::string_view withoutNuls = trimTrailingNuls(text);
    const bool symbolAllowed = withoutNuls.size() == text.size();
    text = trim(withoutNuls);
    bool negative = false;
    if (!text.empty() && (text.front() == '+' || text.front() == '-')) {
        negative = text.front() == '-';
        text.remove_prefix(1);
    }
    if (text.empty()) return false;

    // The invariant culture's symbols, matched case-insensitively ("∞" is not one of them).
    if (symbolAllowed && equalsIgnoreCase(text, "Infinity")) {
        value = negative ? -std::numeric_limits<float>::infinity()
                         : std::numeric_limits<float>::infinity();
        return true;
    }
    if (symbolAllowed && equalsIgnoreCase(text, "NaN")) {
        // float.NaN, whatever the sign: the x86 default NaN, bits 0xFFC00000.
        value = std::copysign(std::numeric_limits<float>::quiet_NaN(), -1.0f);
        return true;
    }

    // Only digits, one '.', and an exponent are allowed past this point (from_chars would also
    // take "inf"/"nan" spellings and hex, which .NET does not).
    bool sawDigit = false;
    bool sawPoint = false;
    std::size_t i = 0;
    for (; i < text.size(); ++i) {
        const char c = text[i];
        if (c >= '0' && c <= '9') {
            sawDigit = true;
        } else if (c == '.' && !sawPoint) {
            sawPoint = true;
        } else {
            break;
        }
    }
    if (!sawDigit) return false;
    if (i < text.size()) {
        if (text[i] != 'e' && text[i] != 'E') return false;
        std::size_t j = i + 1;
        if (j < text.size() && (text[j] == '+' || text[j] == '-')) ++j;
        if (j >= text.size()) return false;
        for (; j < text.size(); ++j) {
            if (text[j] < '0' || text[j] > '9') return false;
        }
    }

    float parsed = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), parsed);
    if (ec == std::errc::result_out_of_range) {
        // .NET Core 3.0+ saturates to infinity / rounds to zero instead of failing.
        parsed = static_cast<float>(std::strtod(std::string(text).c_str(), nullptr));
    } else if (ec != std::errc() || ptr != text.data() + text.size()) {
        return false;
    }
    value = negative ? -parsed : parsed;
    return true;
}

}  // namespace Sarwave::Core::MMWave::DotNet
