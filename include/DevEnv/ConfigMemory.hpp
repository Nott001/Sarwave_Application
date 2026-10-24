#pragma once

#include <optional>
#include <string>

namespace Sarwave::DevEnv {

/// What the dev environment remembers between runs: the last .cfg file applied and the LAN
/// addresses last used. Kept in "config.json" in the working directory, e.g.
/// {"CfgPath": "...", "LanListenHost": "0.0.0.0", "LanListenPort": 5577}, written exactly the
/// way the C# version's System.Text.Json does (two-space indent, same escaping, members in the
/// order they were added), so the file is shared between the two. Saving one setting keeps the
/// others, unknown ones included.
struct ConfigMemory {
    static constexpr const char* CfgFile = "config.json";

    /// The remembered cfg path; nullopt when there is none (no config.json, unreadable JSON, no
    /// "CfgPath", or JSON null). Throws InvalidOperationException when it is not a string.
    static std::optional<std::string> getConfig();

    static void saveConfigLocation(const std::string& value);

    struct LanAddress {
        std::string host;
        int port = 0;
    };

    /// sending: the live app's listening address if true; the wireless app's sender if false.
    /// nullopt unless both the host and the port are remembered. Throws InvalidOperationException
    /// when they are not a string and an int.
    static std::optional<LanAddress> getLanAddress(bool sending);

    /// sending: see getLanAddress.
    static void saveLanAddress(bool sending, const std::string& host, int port);

    /// The JSON text saveConfigLocation writes when there is no config.json yet (for tests).
    static std::string serialize(const std::string& value);
};

}  // namespace Sarwave::DevEnv
