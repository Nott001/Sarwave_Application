#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <istream>
#include <memory>
#include <mutex>
#include <ostream>
#include <span>
#include <vector>

#include "Imu/Fusion/ImuProcessor.hpp"
#include "Imu/ImuSample.hpp"
#include "Imu/Timeline/ImuTimeline.hpp"

namespace Sarwave::Core::Imu::Recording {

/// On-disk layout shared by ImuRecorder and ImuRecording -- byte for byte the C# format, so
/// recordings move freely between the two applications. Everything is little endian.
///
///   File header (16 bytes):  8-byte signature "SWIMUREC", int32 version, float32 nominal
///                            sample rate in Hz
///   Then one 36-byte record per sample:
///     int64      timestamp, in TimeSpan ticks, on the radar frames' clock
///     float32 ×3 acceleration, g, IMU axes
///     float32 ×3 angular rate, °/s, IMU axes, bias not removed
///     float32    temperature, °C
///
/// Samples are raw, so a recording can be processed again with a corrected mounting or new
/// settings.
struct ImuRecordingFormat {
    static constexpr std::int32_t Version = 1;
    static constexpr int FileHeaderSize = 16;
    static constexpr int RecordSize = 8 + 7 * 4;
    static constexpr std::array<std::uint8_t, 8> Signature{'S', 'W', 'I', 'M', 'U', 'R', 'E', 'C'};

    static void writeFileHeader(std::span<std::uint8_t> destination, float sampleRate);
    static void writeRecord(std::span<std::uint8_t> destination, const ImuSample& sample);
    static ImuSample readRecord(std::span<const std::uint8_t> source);
};

/// Writes raw IMU samples to a file, to be loaded again with ImuRecording. It goes next to a
/// radar frame recording (see ImuRecording::sidecarPath) and shares its clock, so every recorded
/// frame keeps the IMU data it was gated and levelled with, as the thesis's data collection
/// protocol asks.
///
/// Its record() fits ImuSampler::BatchCallback. Each batch reaches the operating system as soon
/// as it is recorded, so a crash loses nothing already recorded. Safe to call from the sampler
/// thread while another thread disposes it.
class ImuRecorder {
   public:
    /// Creates the file, replacing any existing one. Throws IOException if it can't.
    /// sampleRate: nominal sample rate, Hz; stored for reference only.
    ImuRecorder(const std::filesystem::path& path, float sampleRate);

    /// Writes to `stream`, left open when this recorder is disposed.
    ImuRecorder(std::ostream& stream, float sampleRate);

    ~ImuRecorder();
    ImuRecorder(const ImuRecorder&) = delete;
    ImuRecorder& operator=(const ImuRecorder&) = delete;

    /// Samples written so far.
    [[nodiscard]] std::int64_t sampleCount() const;

    /// Throws ObjectDisposedException after dispose(), IOException when the write fails.
    void record(std::span<const ImuSample> samples);

    void dispose();

   private:
    void write(std::span<const std::uint8_t> bytes);

    mutable std::mutex m_gate;
    std::unique_ptr<std::ostream> m_owned;
    std::ostream* m_stream;
    std::vector<std::uint8_t> m_buffer;
    bool m_disposed = false;
    std::int64_t m_sampleCount = 0;
};

/// A whole IMU recording loaded into memory, oldest sample first. Turn it into a timeline with
/// toTimeline() to gate and level replayed radar frames exactly as the live run did.
class ImuRecording {
   public:
    static constexpr const char* SidecarExtension = ".swimu";

    [[nodiscard]] std::span<const ImuSample> samples() const {
        return m_samples;
    }

    [[nodiscard]] int count() const {
        return static_cast<int>(m_samples.size());
    }

    /// Nominal sample rate the recording was made at, Hz.
    [[nodiscard]] float sampleRate() const {
        return m_sampleRate;
    }

    /// True if the file ended part-way through a sample (the recorder was killed mid-write).
    [[nodiscard]] bool isTruncated() const {
        return m_isTruncated;
    }

    /// Where the IMU recording for a radar frame recording lives: same name, SidecarExtension.
    static std::filesystem::path sidecarPath(const std::filesystem::path& frameRecordingPath);

    /// Throws FileNotFoundException for a missing file, InvalidDataException for something that
    /// is not an IMU recording or an unknown version.
    static ImuRecording load(const std::filesystem::path& path);

    /// Reads from the stream's current position to its end.
    static ImuRecording load(std::istream& stream);

    /// Runs every sample through a fresh ImuProcessor into a timeline big enough to hold them
    /// all, ready for a MotionGate.
    [[nodiscard]] std::unique_ptr<Timeline::ImuTimeline> toTimeline(
        Fusion::ImuMounting mounting, Fusion::ImuProcessorSettings settings = {}) const;

   private:
    ImuRecording(std::vector<ImuSample> samples, float sampleRate, bool isTruncated)
        : m_samples(std::move(samples)), m_sampleRate(sampleRate), m_isTruncated(isTruncated) {
    }

    std::vector<ImuSample> m_samples;
    float m_sampleRate = 0;
    bool m_isTruncated = false;
};

}  // namespace Sarwave::Core::Imu::Recording
