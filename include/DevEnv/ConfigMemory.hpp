#pragma once

#include <optional>
#include <string>

namespace Sarwave::DevEnv {

/// The one setting the dev environment remembers between runs: the last .cfg file applied. Kept
/// in "config.json" in the working directory as {"CfgPath": "..."}, written exactly the way the
/// C# version's System.Text.Json does (two-space indent, same escaping), so the file is shared
/// between the two.
struct ConfigMemory {
    static constexpr const char* CfgFile = "config.json";

    /// The remembered path; nullopt when there is no config.json or it has no "CfgPath"
    /// (or it is JSON null). Throws std::runtime_error for malformed JSON.
    static std::optional<std::string> getConfig();

    static void saveConfigLocation(const std::string& value);

    /// The JSON text saveConfigLocation writes (exposed for tests).
    static std::string serialize(const std::string& value);
};

}  // namespace Sarwave::DevEnv
