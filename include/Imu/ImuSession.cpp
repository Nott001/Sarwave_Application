#include "Imu/ImuSession.hpp"

#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::Core::Imu {

ImuSession::ImuSession(std::unique_ptr<Device::Icm42688> device, const ImuSessionOptions& options)
    : m_device(std::move(device)),
      m_options(options),
      m_processor(options.mounting, options.processing),
      m_timeline(options.timelineCapacity),
      m_sampler(*m_device, m_processor, m_timeline),
      m_gate(m_timeline, options.gate) {
}

std::unique_ptr<ImuSession> ImuSession::open(const ImuSessionOptions& options) {
    return std::unique_ptr<ImuSession>(
        new ImuSession(Device::Icm42688::open(options.device), options));
}

std::unique_ptr<ImuSession> ImuSession::tryOpen(const ImuSessionOptions& options,
                                                std::string& reason) {
    try {
        reason.clear();
        return open(options);
    } catch (const std::exception& ex) {
        reason = ex.what();
        return nullptr;
    }
}

ImuSession::~ImuSession() {
    dispose();
}

void ImuSession::start(Sampling::ImuSampler::Clock clock,
                       const std::optional<std::filesystem::path>& recordingPath,
                       Sampling::ImuSampler::BatchCallback onBatch) {
    if (m_disposed) throw ObjectDisposedException("Sarwave.Core.Imu.ImuSession");
    // Checked before the recorder exists: creating it would already have emptied the file.
    if (m_sampler.isRunning())
        throw InvalidOperationException("The IMU session is already sampling.");

    std::unique_ptr<Recording::ImuRecorder> recorder;
    if (recordingPath)
        recorder =
            std::make_unique<Recording::ImuRecorder>(*recordingPath, m_options.device.rateHz());

    Sampling::ImuSampler::BatchCallback callback = std::move(onBatch);
    if (recorder) {
        callback = [r = recorder.get(),
                    also = std::move(callback)](std::span<const ImuSample> samples) {
            if (also) also(samples);
            r->record(samples);
        };
    }
    m_sampler.start(std::move(clock), std::move(callback));  // on failure `recorder` closes itself

    m_recorder = std::move(recorder);
}

void ImuSession::dispose() {
    if (m_disposed) return;
    m_disposed = true;

    m_sampler.stop();
    if (m_recorder) m_recorder->dispose();
    m_device->dispose();
}

}  // namespace Sarwave::Core::Imu
