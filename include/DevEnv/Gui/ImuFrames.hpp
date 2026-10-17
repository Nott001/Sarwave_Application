#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "Gui/Models/RadarData.hpp"
#include "Imu/Gating/MotionGate.hpp"
#include "Imu/ImuSession.hpp"
#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Streaming/Frame.hpp"

namespace Sarwave::DevEnv::Gui {

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

/// The IMU's part in the dashboards, shared by the live and the recorded app so a replayed
/// session is gated and levelled exactly as it was live.
struct ImuFrames {
    /// How the ICM-42688-P is wired and mounted, and the gating thresholds. The mounting assumes
    /// the IMU's X arrow points to the radar's right, Y along the boresight and Z up; if it sits
    /// otherwise, say so with ImuMounting::fromAxes (tilting the radar's nose up must read as
    /// positive pitch).
    static Core::Imu::ImuSessionOptions& options();

    /// Whether the live app uses the IMU. Read when the app opens.
    static ImuMode mode;

    /// Opens the IMU as `mode` says. Null means the IMU is disabled for this session: the radar
    /// runs without motion gating or tilt compensation, and nothing touches SPI.
    /// Throws InvalidOperationException for ImuMode::Required when no IMU was detected.
    static std::unique_ptr<Core::Imu::ImuSession> tryOpen();

    /// The gate for replaying `frameRecordingPath`, built from the IMU recording beside it;
    /// nullopt if there is none (a session recorded without the IMU) or it is unusable.
    static std::optional<RecordedGate> loadRecordedGate(
        const std::filesystem::path& frameRecordingPath);

    /// Hands a frame to the dashboard as the IMU says: levelled if accepted, skipped (and the
    /// operator asked to hold steady) if refused, and as it is without IMU data.
    ///
    /// Throws FrameEntryMalformedException when the frame is malformed, whether or not the IMU
    /// refused it, so a malformed frame is reported as one even while the device moves.
    /// imu: the gate's verdict on this frame; nullopt without an IMU.
    static void apply(Core::Gui::Models::RadarData& data,
                      const Core::MMWave::Streaming::Frame& frame, Core::MMWave::TimeSpan timestamp,
                      const std::optional<Core::Imu::Gating::ImuFrameState>& imu);
};

}  // namespace Sarwave::DevEnv::Gui
