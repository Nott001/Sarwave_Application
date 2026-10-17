#include "DevEnv/Gui/ImuFrames.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>

#include "Imu/Recording/ImuRecording.hpp"
#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Configured/PeopleTracking/Association/CombinedManager.hpp"

namespace Sarwave::DevEnv::Gui {

using namespace Sarwave::Core;

ImuMode ImuFrames::mode = ImuMode::Auto;

std::optional<ImuMode> parseImuMode(const std::string& text) {
    std::string lower = text;
    std::ranges::transform(lower, lower.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (lower == "auto") return ImuMode::Auto;
    if (lower == "off") return ImuMode::Off;
    if (lower == "required") return ImuMode::Required;
    return std::nullopt;
}

Imu::ImuSessionOptions& ImuFrames::options() {
    static Imu::ImuSessionOptions options;
    return options;
}

std::unique_ptr<Imu::ImuSession> ImuFrames::tryOpen() {
    if (mode == ImuMode::Off) {
        std::cerr << "IMU disabled by setting: no motion gating or tilt compensation." << std::endl;
        return nullptr;
    }

    std::string reason;
    auto session = Imu::ImuSession::tryOpen(options(), reason);
    if (session) return session;

    if (mode == ImuMode::Required)
        throw InvalidOperationException("IMU required but not detected: " + reason);

    std::cerr << "IMU not detected, disabled: no motion gating or tilt compensation. (" << reason
              << ")" << std::endl;
    return nullptr;
}

std::optional<RecordedGate> ImuFrames::loadRecordedGate(
    const std::filesystem::path& frameRecordingPath) {
    const std::filesystem::path path =
        Imu::Recording::ImuRecording::sidecarPath(frameRecordingPath);
    if (!std::filesystem::exists(path)) return std::nullopt;

    try {
        const auto recording = Imu::Recording::ImuRecording::load(path);
        RecordedGate result;
        result.timeline = recording.toTimeline(options().mounting, options().processing);
        result.gate = std::make_unique<Imu::Gating::MotionGate>(*result.timeline, options().gate);
        return result;
    } catch (const InvalidDataException& ex) {
        std::cerr << "IMU recording unusable, replaying without motion gating: " << ex.what()
                  << std::endl;
        return std::nullopt;
    }
}

void ImuFrames::apply(Core::Gui::Models::RadarData& data, const MMWave::Streaming::Frame& frame,
                      MMWave::TimeSpan timestamp,
                      const std::optional<Imu::Gating::ImuFrameState>& imu) {
    data.setHoldSteady(imu && imu->holdSteady());

    if (imu && !imu->accepted) {
        // Checked although unused: a refused frame would otherwise never be unpacked, and a
        // malformed one would go unreported and be recorded as intact.
        (void)MMWave::Configured::PeopleTracking::Association::EntryPackage::unpack(frame);
        data.skipFrame(timestamp);
        return;
    }

    data.update(frame, timestamp, imu ? imu->tilt.rotation() : MMWave::Quaternion::identity());
}

}  // namespace Sarwave::DevEnv::Gui
