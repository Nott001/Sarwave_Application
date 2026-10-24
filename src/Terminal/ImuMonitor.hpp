#pragma once

#include <optional>
#include <string>

/// The IMU on its own, live in the terminal: what it reads, the tilt and motion it makes of that,
/// and what the motion gate would say about a radar frame right now. For checking the mounting
/// (docs/imu_mounting_checks.md) and the gate's thresholds without the radar connected.
struct ImuMonitor {
    /// recordingPath: where to record the raw samples, for imu_check; nullopt records nothing.
    /// Returns the process exit code.
    static int run(const std::optional<std::string>& recordingPath);
};
