#include "Imu/Recording/ImuRecording.hpp"

#include <algorithm>
#include <bit>
#include <fstream>

#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::Core::Imu::Recording {
namespace {

void writeLe(std::span<std::uint8_t> to, std::uint64_t value, int bytes) {
    for (int i = 0; i < bytes; ++i)
        to[static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(value >> (8 * i));
}

std::uint64_t readLe(std::span<const std::uint8_t> from, int bytes) {
    std::uint64_t value = 0;
    for (int i = 0; i < bytes; ++i)
        value |= static_cast<std::uint64_t>(from[static_cast<std::size_t>(i)]) << (8 * i);
    return value;
}

void writeSingle(std::span<std::uint8_t> to, float value) {
    writeLe(to, std::bit_cast<std::uint32_t>(value), 4);
}

float readSingle(std::span<const std::uint8_t> from) {
    return std::bit_cast<float>(static_cast<std::uint32_t>(readLe(from, 4)));
}

void writeVector(std::span<std::uint8_t> to, Vector3 v) {
    writeSingle(to, v.x);
    writeSingle(to.subspan(4), v.y);
    writeSingle(to.subspan(8), v.z);
}

Vector3 readVector(std::span<const std::uint8_t> from) {
    return {readSingle(from), readSingle(from.subspan(4)), readSingle(from.subspan(8))};
}

std::size_t readSome(std::istream& stream, std::span<std::uint8_t> destination) {
    stream.read(reinterpret_cast<char*>(destination.data()),
                static_cast<std::streamsize>(destination.size()));
    return static_cast<std::size_t>(stream.gcount());
}

}  // namespace

// ---- format -----------------------------------------------------------------------------------

void ImuRecordingFormat::writeFileHeader(std::span<std::uint8_t> destination, float sampleRate) {
    std::copy(Signature.begin(), Signature.end(), destination.begin());
    writeLe(destination.subspan(8), static_cast<std::uint32_t>(Version), 4);
    writeSingle(destination.subspan(12), sampleRate);
}

void ImuRecordingFormat::writeRecord(std::span<std::uint8_t> destination, const ImuSample& sample) {
    writeLe(destination, static_cast<std::uint64_t>(sample.timestamp.count()), 8);
    writeVector(destination.subspan(8), sample.acceleration);
    writeVector(destination.subspan(20), sample.angularRate);
    writeSingle(destination.subspan(32), sample.temperature);
}

ImuSample ImuRecordingFormat::readRecord(std::span<const std::uint8_t> source) {
    return {TimeSpan(static_cast<std::int64_t>(readLe(source, 8))), readVector(source.subspan(8)),
            readVector(source.subspan(20)), readSingle(source.subspan(32))};
}

// ---- recorder ---------------------------------------------------------------------------------

ImuRecorder::ImuRecorder(const std::filesystem::path& path, float sampleRate) {
    auto file = std::make_unique<std::ofstream>(path, std::ios::binary | std::ios::trunc);
    if (!file->is_open()) {
        throw IOException("Could not create file '" + std::filesystem::absolute(path).string() +
                          "'.");
    }
    m_owned = std::move(file);
    m_stream = m_owned.get();

    std::array<std::uint8_t, ImuRecordingFormat::FileHeaderSize> header{};
    ImuRecordingFormat::writeFileHeader(header, sampleRate);
    write(header);
    m_stream->flush();
}

ImuRecorder::ImuRecorder(std::ostream& stream, float sampleRate) : m_stream(&stream) {
    std::array<std::uint8_t, ImuRecordingFormat::FileHeaderSize> header{};
    ImuRecordingFormat::writeFileHeader(header, sampleRate);
    write(header);
}

ImuRecorder::~ImuRecorder() {
    dispose();
}

std::int64_t ImuRecorder::sampleCount() const {
    std::lock_guard lock(m_gate);
    return m_sampleCount;
}

void ImuRecorder::write(std::span<const std::uint8_t> bytes) {
    m_stream->write(reinterpret_cast<const char*>(bytes.data()),
                    static_cast<std::streamsize>(bytes.size()));
    if (!*m_stream) throw IOException("Could not write to the IMU recording.");
}

void ImuRecorder::record(std::span<const ImuSample> samples) {
    if (samples.empty()) return;

    std::lock_guard lock(m_gate);
    if (m_disposed) throw ObjectDisposedException("Sarwave.Core.Imu.Recording.ImuRecorder");

    m_buffer.resize(samples.size() * ImuRecordingFormat::RecordSize);
    for (std::size_t i = 0; i < samples.size(); i++)
        ImuRecordingFormat::writeRecord(
            std::span(m_buffer).subspan(i * ImuRecordingFormat::RecordSize), samples[i]);

    write(m_buffer);
    m_stream->flush();
    m_sampleCount += static_cast<std::int64_t>(samples.size());
}

void ImuRecorder::dispose() {
    std::lock_guard lock(m_gate);
    if (m_disposed) return;
    m_disposed = true;

    m_stream->flush();
    m_owned.reset();
}

// ---- recording --------------------------------------------------------------------------------

std::filesystem::path ImuRecording::sidecarPath(const std::filesystem::path& frameRecordingPath) {
    std::filesystem::path path = frameRecordingPath;
    return path.replace_extension(SidecarExtension);
}

ImuRecording ImuRecording::load(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        const std::string full = std::filesystem::absolute(path).lexically_normal().string();
        if (!std::filesystem::exists(path))
            throw FileNotFoundException("Could not find file '" + full + "'.");
        throw IOException("Access to the path '" + full + "' is denied.");
    }
    return load(stream);
}

ImuRecording ImuRecording::load(std::istream& stream) {
    std::array<std::uint8_t, ImuRecordingFormat::FileHeaderSize> header{};
    if (readSome(stream, header) != header.size() ||
        !std::equal(ImuRecordingFormat::Signature.begin(), ImuRecordingFormat::Signature.end(),
                    header.begin())) {
        throw InvalidDataException("Not an IMU recording.");
    }

    const auto version = static_cast<std::int32_t>(
        static_cast<std::uint32_t>(readLe(std::span(header).subspan(8), 4)));
    if (version != ImuRecordingFormat::Version) {
        throw InvalidDataException("Unsupported IMU recording version " + std::to_string(version) +
                                   ".");
    }

    const float sampleRate = readSingle(std::span(header).subspan(12));

    std::vector<ImuSample> samples;
    std::vector<std::uint8_t> buffer(ImuRecordingFormat::RecordSize * 512);
    std::size_t carried = 0;
    bool isTruncated = false;

    while (true) {
        const std::size_t read = readSome(stream, std::span(buffer).subspan(carried));
        const std::size_t available = carried + read;
        const std::size_t whole = available / ImuRecordingFormat::RecordSize;

        for (std::size_t i = 0; i < whole; i++) {
            samples.push_back(ImuRecordingFormat::readRecord(
                std::span(buffer).subspan(i * ImuRecordingFormat::RecordSize)));
        }

        carried = available - whole * ImuRecordingFormat::RecordSize;
        std::copy_n(
            buffer.begin() + static_cast<std::ptrdiff_t>(whole * ImuRecordingFormat::RecordSize),
            carried, buffer.begin());

        if (read == 0) {
            isTruncated = carried != 0;
            break;
        }
    }

    return {std::move(samples), sampleRate, isTruncated};
}

std::unique_ptr<Timeline::ImuTimeline> ImuRecording::toTimeline(
    Fusion::ImuMounting mounting, Fusion::ImuProcessorSettings settings) const {
    Fusion::ImuProcessor processor(mounting, settings);
    auto timeline = std::make_unique<Timeline::ImuTimeline>(std::max(count(), 2));

    constexpr std::size_t Batch = 1024;
    std::vector<ImuState> states(Batch);
    for (std::size_t start = 0; start < m_samples.size(); start += Batch) {
        const std::size_t n = std::min(Batch, m_samples.size() - start);
        for (std::size_t i = 0; i < n; i++) states[i] = processor.process(m_samples[start + i]);
        timeline->append(std::span(states).first(n));
    }

    return timeline;
}

}  // namespace Sarwave::Core::Imu::Recording
