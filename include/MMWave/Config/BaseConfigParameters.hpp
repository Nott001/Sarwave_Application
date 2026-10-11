#pragma once

#include "MMWave/Config/ConfigParameterRegistry.hpp"

namespace Sarwave::Core::MMWave::Config {

/// The commands every mmWave SDK demo shares: sensor control, the radio front end, chirps and
/// frames. A demo-specific reader starts from these and adds its own.
struct BaseConfigParameters {
    /// Registers the shared commands in `registry` and returns it, so further add() calls can
    /// be chained (the C# extension method `registry.AddBaseParameters()`).
    static ConfigParameterRegistry& addBaseParameters(ConfigParameterRegistry& registry);
};

}  // namespace Sarwave::Core::MMWave::Config
