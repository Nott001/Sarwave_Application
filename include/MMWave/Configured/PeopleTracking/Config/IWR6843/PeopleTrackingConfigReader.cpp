#include "MMWave/Configured/PeopleTracking/Config/IWR6843/PeopleTrackingConfigReader.hpp"

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <exception>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>
#include <vector>

#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Config/BaseConfigParameters.hpp"
#include "MMWave/Config/Comment.hpp"
#include "MMWave/Config/ConfigFormatException.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843 {
namespace {

using MMWave::Config::ConfigFormatException;
using MMWave::Config::ConfigParameterPtr;

constexpr char32_t ReplacementChar = 0xFFFD;

// ---- UTF-8 -------------------------------------------------------------------------------------

void appendUtf8(std::string& out, char32_t c) {
    if (c < 0x80) {
        out += static_cast<char>(c);
    } else if (c < 0x800) {
        out += static_cast<char>(0xC0 | (c >> 6));
        out += static_cast<char>(0x80 | (c & 0x3F));
    } else if (c < 0x10000) {
        out += static_cast<char>(0xE0 | (c >> 12));
        out += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (c & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (c >> 18));
        out += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (c & 0x3F));
    }
}

/// One character read from UTF-8 text.
struct Utf8Char {
    char32_t value;
    std::size_t length;
    bool valid;
};

/// Reads the well-formed UTF-8 sequence at `text[i]`, or else the maximal ill-formed subpart
/// there (at least one byte), which .NET's UTF-8 decoder replaces with one U+FFFD.
Utf8Char decodeUtf8(std::string_view text, std::size_t i) {
    const auto byte = [&](std::size_t k) { return static_cast<std::uint8_t>(text[i + k]); };
    const std::uint8_t lead = byte(0);
    if (lead < 0x80) return {lead, 1, true};

    std::size_t length = 0;
    std::uint8_t secondMin = 0x80;
    std::uint8_t secondMax = 0xBF;
    if (lead >= 0xC2 && lead <= 0xDF) {
        length = 2;
    } else if (lead >= 0xE0 && lead <= 0xEF) {
        length = 3;
        if (lead == 0xE0) secondMin = 0xA0;  // no overlong forms
        if (lead == 0xED) secondMax = 0x9F;  // no surrogates
    } else if (lead >= 0xF0 && lead <= 0xF4) {
        length = 4;
        if (lead == 0xF0) secondMin = 0x90;  // no overlong forms
        if (lead == 0xF4) secondMax = 0x8F;  // nothing above U+10FFFF
    } else {
        return {ReplacementChar, 1, false};
    }

    char32_t value = lead & (0x7F >> length);
    for (std::size_t k = 1; k < length; k++) {
        const std::uint8_t min = k == 1 ? secondMin : 0x80;
        const std::uint8_t max = k == 1 ? secondMax : 0xBF;
        if (i + k >= text.size() || byte(k) < min || byte(k) > max) {
            return {ReplacementChar, k, false};
        }
        value = (value << 6) | (byte(k) & 0x3F);
    }

    return {value, length, true};
}

/// char.IsWhiteSpace.
bool isWhiteSpace(char32_t c) {
    return (c >= 0x09 && c <= 0x0D) || c == 0x20 || c == 0x85 || c == 0xA0 || c == 0x1680 ||
           (c >= 0x2000 && c <= 0x200A) || c == 0x2028 || c == 0x2029 || c == 0x202F ||
           c == 0x205F || c == 0x3000;
}

/// string.TrimEnd(): drops trailing white space.
std::string_view trimEnd(std::string_view text) {
    std::size_t end = 0;
    for (std::size_t i = 0; i < text.size();) {
        const Utf8Char c = decodeUtf8(text, i);
        i += c.length;
        if (!isWhiteSpace(c.value)) end = i;
    }

    return text.substr(0, end);
}

/// string.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries): the words between runs of
/// white space.
std::vector<std::string> splitWords(std::string_view text) {
    std::vector<std::string> words;
    std::size_t start = 0;
    for (std::size_t i = 0; i < text.size();) {
        const Utf8Char c = decodeUtf8(text, i);
        if (isWhiteSpace(c.value)) {
            if (i > start) words.emplace_back(text.substr(start, i - start));
            start = i + c.length;
        }
        i += c.length;
    }
    if (text.size() > start) words.emplace_back(text.substr(start));

    return words;
}

// ---- StreamReader's decoding -------------------------------------------------------------------

std::string fromUtf8(std::string_view bytes) {
    std::string text;
    text.reserve(bytes.size());
    for (std::size_t i = 0; i < bytes.size();) {
        const Utf8Char c = decodeUtf8(bytes, i);
        if (c.valid) {
            text.append(bytes.substr(i, c.length));
        } else {
            appendUtf8(text, ReplacementChar);
        }
        i += c.length;
    }

    return text;
}

std::string fromUtf16(std::string_view bytes, bool bigEndian) {
    const auto unitAt = [&](std::size_t i) -> char32_t {
        const auto first = static_cast<std::uint8_t>(bytes[i]);
        const auto second = static_cast<std::uint8_t>(bytes[i + 1]);
        return bigEndian ? (first << 8) | second : (second << 8) | first;
    };

    std::string text;
    std::size_t i = 0;
    for (; i + 1 < bytes.size(); i += 2) {
        const char32_t unit = unitAt(i);
        if (unit >= 0xD800 && unit <= 0xDBFF && i + 3 < bytes.size()) {
            const char32_t low = unitAt(i + 2);
            if (low >= 0xDC00 && low <= 0xDFFF) {
                appendUtf8(text, 0x10000 + ((unit - 0xD800) << 10) + (low - 0xDC00));
                i += 2;
                continue;
            }
        }
        appendUtf8(text, unit >= 0xD800 && unit <= 0xDFFF ? ReplacementChar : unit);
    }
    if (i < bytes.size()) appendUtf8(text, ReplacementChar);  // an odd byte at the end

    return text;
}

std::string fromUtf32(std::string_view bytes, bool bigEndian) {
    std::string text;
    std::size_t i = 0;
    for (; i + 3 < bytes.size(); i += 4) {
        char32_t c = 0;
        for (std::size_t k = 0; k < 4; k++) {
            const auto b = static_cast<std::uint8_t>(bytes[i + (bigEndian ? k : 3 - k)]);
            c = (c << 8) | b;
        }
        const bool valid = c <= 0x10FFFF && (c < 0xD800 || c > 0xDFFF);
        appendUtf8(text, valid ? c : ReplacementChar);
    }
    if (i < bytes.size()) appendUtf8(text, ReplacementChar);

    return text;
}

/// StreamReader with detectEncodingFromByteOrderMarks: the byte order mark picks the encoding
/// and is dropped; without one the bytes are UTF-8.
std::string decodeLikeStreamReader(std::string_view bytes) {
    if (bytes.starts_with("\xFE\xFF")) return fromUtf16(bytes.substr(2), true);
    if (bytes.starts_with("\xFF\xFE")) {
        if (bytes.size() >= 4 && bytes[2] == 0 && bytes[3] == 0) {
            return fromUtf32(bytes.substr(4), false);
        }
        return fromUtf16(bytes.substr(2), false);
    }
    if (bytes.starts_with("\xEF\xBB\xBF")) return fromUtf8(bytes.substr(3));
    if (bytes.starts_with(std::string_view("\0\0\xFE\xFF", 4))) {
        return fromUtf32(bytes.substr(4), true);
    }

    return fromUtf8(bytes);
}

// ---- opening the file the way FileStream does --------------------------------------------------

/// Path.GetFullPath: relative to the working directory, with "." and ".." segments and repeated
/// separators collapsed.
std::filesystem::path fullPath(const std::filesystem::path& path) {
    return std::filesystem::absolute(path).lexically_normal();
}

[[noreturn]] void throwForError(std::error_code error, const std::filesystem::path& path) {
    const std::string shown = path.string();
    if (error == std::errc::no_such_file_or_directory) {
        // As .NET does: the file is missing only if its directory exists; otherwise it is the
        // directory that is missing.
        std::filesystem::path trimmed = path;
        if (!trimmed.has_filename() && trimmed.has_relative_path()) trimmed = trimmed.parent_path();
        std::error_code ignored;
        if (std::filesystem::is_directory(trimmed.parent_path(), ignored)) {
            throw FileNotFoundException("Could not find file '" + shown + "'.");
        }
        throw DirectoryNotFoundException("Could not find a part of the path '" + shown + "'.");
    }
    if (error == std::errc::not_a_directory) {
        throw DirectoryNotFoundException("Could not find a part of the path '" + shown + "'.");
    }
    if (error == std::errc::permission_denied || error == std::errc::operation_not_permitted) {
        throw UnauthorizedAccessException("Access to the path '" + shown + "' is denied.");
    }
    throw IOException(error.message() + " : '" + shown + "'");
}

std::string readAllBytes(const std::filesystem::path& cfgPath) {
    if (cfgPath.empty()) throw ArgumentException("The value cannot be an empty string.", "path");

    const std::filesystem::path path = fullPath(cfgPath);
    std::error_code error;
    const std::filesystem::file_status status = std::filesystem::status(path, error);
    if (error) throwForError(error, path);
    if (std::filesystem::is_directory(status)) {
        throwForError(std::make_error_code(std::errc::permission_denied), path);
    }

    errno = 0;
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throwForError(std::error_code(errno != 0 ? errno : EIO, std::generic_category()), path);
    }

    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

}  // namespace

MMWave::Config::ConfigParameterRegistry& PeopleTrackingConfigReader::registry() {
    static MMWave::Config::ConfigParameterRegistry instance = [] {
        using namespace Parameters;
        MMWave::Config::ConfigParameterRegistry registry;
        MMWave::Config::BaseConfigParameters::addBaseParameters(registry)
            // detection layer
            .add<DynamicRangeAngleCfg>()
            .add<DynamicRACfarCfg>()
            .add<Dynamic2DAngleCfg>()
            .add<StaticRangeAngleCfg>()
            .add<StaticRACfarCfg>()
            .add<FineMotionCfg>()
            // board
            .add<AntGeometry0>()
            .add<AntGeometry1>()
            .add<AntPhaseRot>()
            .add<FovCfg>()
            // tracker layer
            .add<StaticBoundaryBox>()
            .add<BoundaryBox>()
            .add<PresenceBoundaryBox>()
            .add<SensorPosition>()
            .add<GatingParam>()
            .add<StateParam>()
            .add<AllocationParam>()
            .add<MaxAcceleration>()
            .add<TrackingCfg>();
        return registry;
    }();
    return instance;
}

PeopleTrackingConfig PeopleTrackingConfigReader::readFile(const std::filesystem::path& cfgPath) {
    return parse(decodeLikeStreamReader(readAllBytes(cfgPath)));
}

PeopleTrackingConfig PeopleTrackingConfigReader::read(std::istream& reader) {
    const std::string text{std::istreambuf_iterator<char>(reader),
                           std::istreambuf_iterator<char>()};
    return parse(text);
}

PeopleTrackingConfig PeopleTrackingConfigReader::parse(std::string_view cfgText) {
    std::vector<ConfigParameterPtr> parameters;
    int lineNumber = 0;

    std::size_t position = 0;
    while (position < cfgText.size()) {
        // TextReader.ReadLine: a line ends at "\n", "\r\n" or a lone "\r".
        const std::size_t end = cfgText.find_first_of("\r\n", position);
        std::string_view raw = cfgText.substr(position, end - position);
        if (end == std::string_view::npos) {
            position = cfgText.size();
        } else {
            position = end + (cfgText.substr(end, 2) == "\r\n" ? 2 : 1);
        }

        lineNumber++;
        const std::string_view line = trimEnd(raw);

        if (line.empty()) {
            parameters.push_back(MMWave::Config::Comment::blank());
            continue;
        }

        if (line[0] == MMWave::Config::Comment::Marker) {
            parameters.push_back(
                std::make_shared<const MMWave::Config::Comment>(std::string(line.substr(1))));
            continue;
        }

        std::vector<std::string> words = splitWords(line);
        std::string paramName = std::move(words[0]);
        words.erase(words.begin());
        try {
            parameters.push_back(registry().parse(paramName, std::move(words)));
        } catch (const ConfigFormatException& ex) {
            std::throw_with_nested(ConfigFormatException(ex.message(), lineNumber));
        }
    }

    return PeopleTrackingConfig(std::move(parameters));
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843
