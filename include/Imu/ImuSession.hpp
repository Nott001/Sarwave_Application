#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "Imu/Device/Icm42688.hpp"
#include "Imu/Fusion/ImuProcessor.hpp"
#include "Imu/Gating/MotionGate.hpp"
#include "Imu/Recording/ImuRecording.hpp"
#include "Imu/Sampling/ImuSampler.hpp"
#include "Imu/Timeline/ImuTimeline.hpp"

namespace Sarwave::Core::Imu {

/// Everything an ImuSession needs; every part has working defaults.
struct ImuSessionOptions {
    Device::Icm42688Settings device;
    Fusion::ImuMounting mounting = Fusion::ImuMounting::identity();
    Fusion::ImuProcessorSettings processing;
    Gating::MotionGateSettings gate;

    /// States kept for pairing with frames. 4096 is 20 s at 200 Hz.
    int timelineCapacity = 4096;
};

/// The live IMU side of the handheld radar, assembled: the ICM-42688-P on SPI, a sampler thread
/// filling a timeline, and the motion gate that the radar's frame callback asks about each
/// frame.
///
/// Typical use, next to a ConstantCapture:
///
///   auto imu = ImuSession::open();
///   capture.beginConstantCapture([&](const Frame& frame, TimeSpan t) {
///       ImuFrameState state = imu->evaluate(t);
///       if (!state.accepted) { /* hold steady: skip classification */ return; }
///       /* level the points with state.tilt, then classify */
///   });
///   imu->start([&] { return capture.elapsed(); }, ImuRecording::sidecarPath(recordingPath));
class ImuSession {
   public:
    /// Opens and configures the IMU. Sampling starts with start().
    /// Throws ImuDeviceException when no ICM-42688-P answered or it would not take its
    /// configuration, IOException when the SPI device could not be opened.
    static std::unique_ptr<ImuSession> open(const ImuSessionOptions& options = {});

    /// Opens the IMU if one can be found, for running without it otherwise. Any reason it
    /// cannot be used counts as not detected: nothing on the chip select, no SPI device (SPI
    /// disabled, or not a Raspberry Pi), no permission, or a platform without Linux SPI at all.
    /// reason: why there is no session, when there is none.
    static std::unique_ptr<ImuSession> tryOpen(const ImuSessionOptions& options,
                                               std::string& reason);

    ~ImuSession();
    ImuSession(const ImuSession&) = delete;
    ImuSession& operator=(const ImuSession&) = delete;

    [[nodiscard]] const ImuSessionOptions& options() const {
        return m_options;
    }
    Fusion::ImuProcessor& processor() {
        return m_processor;
    }
    Timeline::ImuTimeline& timeline() {
        return m_timeline;
    }
    Sampling::ImuSampler& sampler() {
        return m_sampler;
    }
    Gating::MotionGate& gate() {
        return m_gate;
    }

    /// Starts sampling.
    /// clock: the radar frames' clock, already running, e.g. `[&] { return capture.elapsed(); }`.
    /// recordingPath: where to record the raw samples; nullopt records nothing.
    /// onBatch: also receives each batch of raw samples, on the sampler thread, before the
    /// recording does; e.g. a sender streaming them elsewhere. If either throws, both stop
    /// receiving batches.
    void start(Sampling::ImuSampler::Clock clock,
               const std::optional<std::filesystem::path>& recordingPath = std::nullopt,
               Sampling::ImuSampler::BatchCallback onBatch = {});

    /// The IMU's verdict on a radar frame: whether to classify it and how to level its points.
    /// Call once per frame, in order, from the frame callback.
    /// frameTimestamp: the frame's timestamp, on the clock given to start().
    Gating::ImuFrameState evaluate(TimeSpan frameTimestamp) {
        return m_gate.evaluate(frameTimestamp);
    }

    /// Stops sampling, closes the recording and powers the IMU down.
    void dispose();

   private:
    ImuSession(std::unique_ptr<Device::Icm42688> device, const ImuSessionOptions& options);

    std::unique_ptr<Device::Icm42688> m_device;
    ImuSessionOptions m_options;
    Fusion::ImuProcessor m_processor;
    Timeline::ImuTimeline m_timeline;
    Sampling::ImuSampler m_sampler;
    Gating::MotionGate m_gate;
    std::unique_ptr<Recording::ImuRecorder> m_recorder;
    bool m_disposed = false;
};

}  // namespace Sarwave::Core::Imu
