#pragma once

#include <filesystem>
#include <istream>
#include <string_view>

#include "MMWave/Config/ConfigParameterRegistry.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/PeopleTrackingConfig.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843 {

/// Builds a PeopleTrackingConfig from .cfg text. Line handling matches Context.SendConfigFile: a
/// line starting with "%" is a comment, an empty line is skipped by the device, anything else is
/// a command word followed by whitespace-separated arguments.
///
/// Text is UTF-8. Lines end at "\n", "\r\n" or a lone "\r" (TextReader.ReadLine), and white space
/// is what .NET's char.IsWhiteSpace calls white space, Unicode spaces included.
struct PeopleTrackingConfigReader {
    /// Every command the people tracking demo accepts: the SDK's shared ones plus its own.
    static MMWave::Config::ConfigParameterRegistry& registry();

    /// Reads a .cfg file, decoding it the way .NET's StreamReader does: UTF-8 unless a byte
    /// order mark says UTF-16 or UTF-32, with invalid bytes replaced by U+FFFD.
    ///
    /// Throws FileNotFoundException when there is no file at `cfgPath` (with .NET's message,
    /// "Could not find file '<full path>'."), ConfigFormatException when a known command has
    /// missing, malformed or surplus arguments.
    static PeopleTrackingConfig readFile(const std::filesystem::path& cfgPath);

    /// Reads .cfg text held in a string.
    static PeopleTrackingConfig parse(std::string_view cfgText);

    /// Reads to the end of `reader`; the stream is left open.
    static PeopleTrackingConfig read(std::istream& reader);
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843
