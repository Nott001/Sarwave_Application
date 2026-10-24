#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "Imu/Fusion/Fusion.hpp"
#include "Imu/Gating/MotionGate.hpp"
#include "Imu/ImuSession.hpp"
#include "Integration/PeopleTracking/Association/TrackerFrame.hpp"

namespace Sarwave::Integration::PeopleTracking {

enum class ImuMode {
    /// Use the IMU if it is detected at start-up, otherwise run without it.
    Auto,
    /// Never use the IMU, even if it is connected.
    Off,
    /// Refuse to start without the IMU. For data collection, where every frame must be recorded
    /// with its IMU data.
    Required,
};

/// "auto", "off" or "required" (any case); nullopt for anything else.
std::optional<ImuMode> parseImuMode(const std::string& text);

/// A gate for a replayed session, together with the timeline it reads (built from the session's
/// IMU recording).
struct RecordedGate {
    std::unique_ptr<Core::Imu::Timeline::ImuTimeline> timeline;
    std::unique_ptr<Core::Imu::Gating::MotionGate> gate;
};

/// The IMU's part in the dashboards, shared by the live, the recorded and the wireless app so a
/// replayed or streamed session is gated and levelled exactly as it was live. RadarData::update
/// applies the gate's verdicts.
struct ImuFrames {
    /// The IMU as mounted in the compact enclosure (dslr_camera/compact, c40): the EV_ICM-42688-P
    /// on the inner side wall to the operator's left, chip facing in, header pins down. Its Z
    /// arrow therefore points to the radar's right. Its X forward and Y up assume the board's
    /// arrows read X right and Y away from the header, seen from the chip side; if tilting the
    /// radar's nose up reads as roll instead of positive pitch, they sit otherwise; name them
    /// again here.
    ///
    /// From the model, the AOP chip is 89 mm to the radar's right of the IMU chip, 17 mm ahead
    /// of it and 35 mm above it (taking the IMU chip at the middle of its board).
    static const Core::Imu::Fusion::ImuMounting& handheldMounting();

    /// How the ICM-42688-P is wired and mounted (handheldMounting()), and the gating thresholds.
    /// Trim the mounting's last few degrees with ImuMounting::withLevelCalibration while the
    /// boresight is held level.
    static Core::Imu::ImuSessionOptions& options();

    /// Whether the live app uses the IMU. Read when the app opens.
    static ImuMode mode;

    /// Opens the IMU as `mode` says. Null means the IMU is disabled for this session: the radar
    /// runs without motion gating or tilt compensation, and nothing touches SPI.
    /// Throws InvalidOperationException for ImuMode::Required when no IMU was detected.
    static std::unique_ptr<Core::Imu::ImuSession> tryOpen();

    /// Where the cfg at `cfgPath` tells the device's tracker the sensor is, so targets it stops
    /// reporting can be moved with the device. Without a readable cfg holding a sensorPosition,
    /// targets are taken to be in sensor axes.
    static Association::TrackerFrame loadTrackerFrame(
        const std::optional<std::filesystem::path>& cfgPath);

    /// As loadTrackerFrame(), from cfg text, such as a LAN sender's.
    static Association::TrackerFrame parseTrackerFrame(std::string_view cfgText);

    /// The gate for replaying `frameRecordingPath`, built from the IMU recording beside it;
    /// nullopt if there is none (a session recorded without the IMU) or it is unusable.
    static std::optional<RecordedGate> loadRecordedGate(
        const std::filesystem::path& frameRecordingPath);
};

}  // namespace Sarwave::Integration::PeopleTracking
