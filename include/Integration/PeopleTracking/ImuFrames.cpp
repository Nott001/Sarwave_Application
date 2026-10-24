#include "Integration/PeopleTracking/ImuFrames.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>

#include "Imu/Recording/ImuRecording.hpp"
#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Config/ConfigFormatException.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/PeopleTrackingConfigReader.hpp"

namespace Sarwave::Integration::PeopleTracking {

using namespace Sarwave::Core;
using Association::TrackerFrame;
using MMWave::Configured::PeopleTracking::Config::IWR6843::PeopleTrackingConfig;
using MMWave::Configured::PeopleTracking::Config::IWR6843::PeopleTrackingConfigReader;

namespace {

TrackerFrame fromConfig(const PeopleTrackingConfig& config, const std::string& source) {
    if (const auto* position = config.sensorPosition()) return TrackerFrame::from(*position);

    std::cerr << "No sensorPosition in the cfg (" << source
              << "): dropped targets are moved in sensor axes." << std::endl;
    return TrackerFrame::sensorAxes();
}

}  // namespace

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

const Imu::Fusion::ImuMounting& ImuFrames::handheldMounting() {
    static const Imu::Fusion::ImuMounting mounting = [] {
        auto m = Imu::Fusion::ImuMounting::fromAxes(Imu::Fusion::ImuAxis::PositiveZ,
                                                    Imu::Fusion::ImuAxis::PositiveX,
                                                    Imu::Fusion::ImuAxis::PositiveY);
        m.radarOffset = {0.089f, 0.017f, 0.035f};
        return m;
    }();
    return mounting;
}

Imu::ImuSessionOptions& ImuFrames::options() {
    static Imu::ImuSessionOptions options = [] {
        Imu::ImuSessionOptions o;
        o.mounting = handheldMounting();
        return o;
    }();
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

TrackerFrame ImuFrames::loadTrackerFrame(const std::optional<std::filesystem::path>& cfgPath) {
    try {
        if (cfgPath)
            return fromConfig(PeopleTrackingConfigReader::readFile(*cfgPath), cfgPath->string());

        std::cerr << "No cfg: dropped targets are moved in sensor axes." << std::endl;
    } catch (const IOException& ex) {
        std::cerr << "Cfg unreadable, dropped targets are moved in sensor axes: " << ex.what()
                  << std::endl;
    } catch (const MMWave::Config::ConfigFormatException& ex) {
        std::cerr << "Cfg unreadable, dropped targets are moved in sensor axes: " << ex.what()
                  << std::endl;
    }

    return TrackerFrame::sensorAxes();
}

TrackerFrame ImuFrames::parseTrackerFrame(std::string_view cfgText) {
    try {
        return fromConfig(PeopleTrackingConfigReader::parse(cfgText), "the sender's");
    } catch (const MMWave::Config::ConfigFormatException& ex) {
        std::cerr << "Cfg unreadable, dropped targets are moved in sensor axes: " << ex.what()
                  << std::endl;
        return TrackerFrame::sensorAxes();
    }
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

}  // namespace Sarwave::Integration::PeopleTracking
