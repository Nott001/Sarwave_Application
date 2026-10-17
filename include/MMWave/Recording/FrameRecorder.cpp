#include "MMWave/Recording/FrameRecorder.hpp"

#include <array>
#include <fstream>

#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Recording/RecordingFormat.hpp"

namespace Sarwave::Core::MMWave::Recording {
namespace {

std::unique_ptr<std::ostream> createFile(const std::filesystem::path& path) {
    auto file = std::make_unique<std::ofstream>();
    file->open(path, std::ios::binary | std::ios::trunc);
    if (!file->is_open()) {
        throw IOException("Could not create file '" + std::filesystem::absolute(path).string() +
                          "'.");
    }
    return file;
}

}  // namespace

FrameRecorder::FrameRecorder(const std::filesystem::path& path) : FrameRecorder(createFile(path)) {
}

FrameRecorder::FrameRecorder(std::unique_ptr<std::ostream> stream)
    : m_owned(std::move(stream)), m_stream(m_owned.get()) {
    writeHeader();
}

FrameRecorder::FrameRecorder(std::ostream& stream, bool leaveOpen)
    : m_stream(&stream), m_leaveOpen(leaveOpen) {
    writeHeader();
}

FrameRecorder::~FrameRecorder() {
    dispose();
}

void FrameRecorder::writeHeader() {
    std::array<std::uint8_t, RecordingFormat::FileHeaderSize> header{};
    RecordingFormat::writeFileHeader(header);
    write(header);
}

void FrameRecorder::write(std::span<const std::uint8_t> bytes) {
    m_stream->write(reinterpret_cast<const char*>(bytes.data()),
                    static_cast<std::streamsize>(bytes.size()));
    if (!*m_stream) throw IOException("Could not write to the recording.");
}

void FrameRecorder::throwIfDisposed() const {
    // ObjectDisposedException.ThrowIf(_disposed, this) names the type in full.
    if (m_disposed) throw ObjectDisposedException("Sarwave.Core.MMWave.Recording.FrameRecorder");
}

int FrameRecorder::frameCount() const {
    std::lock_guard lock(m_gate);
    return m_frameCount;
}

int FrameRecorder::malformedCount() const {
    std::lock_guard lock(m_gate);
    return m_malformedCount;
}

void FrameRecorder::record(const Streaming::Frame& frame, TimeSpan timestamp) {
    const auto bytes = frame.bytes();
    std::array<std::uint8_t, RecordingFormat::RecordHeaderSize> header{};
    RecordingFormat::writeRecordHeader(header, timestamp, static_cast<std::int32_t>(bytes.size()));

    std::lock_guard lock(m_gate);
    throwIfDisposed();

    write(header);
    write(bytes);

    // Handed to the operating system after every frame. Recordings are mostly wanted when
    // something went wrong, which is exactly when dispose() may never be reached; with the frames
    // only in this process's buffer, a crash or a kill would lose the last of them, including
    // the frame that caused it.
    m_stream->flush();
    m_frameCount++;
}

void FrameRecorder::recordMalformed(const Exceptions::FrameMalformedException& exception,
                                    TimeSpan timestamp) {
    const std::vector<std::uint8_t> payload = RecordingFormat::encodeMalformed(exception);
    std::array<std::uint8_t, RecordingFormat::RecordHeaderSize> header{};
    RecordingFormat::writeRecordHeader(
        header, timestamp,
        static_cast<std::int32_t>(static_cast<std::uint32_t>(payload.size()) |
                                  RecordingFormat::MalformedFlag));

    std::lock_guard lock(m_gate);
    throwIfDisposed();

    write(header);
    write(payload);
    m_stream->flush();
    m_malformedCount++;
}

void FrameRecorder::flush() {
    std::lock_guard lock(m_gate);
    throwIfDisposed();
    m_stream->flush();
}

void FrameRecorder::dispose() {
    std::lock_guard lock(m_gate);
    if (m_disposed) return;
    m_disposed = true;

    m_stream->flush();
    if (!m_leaveOpen && m_owned) {
        if (auto* file = dynamic_cast<std::ofstream*>(m_owned.get())) file->close();
        m_owned.reset();
    }
}

}  // namespace Sarwave::Core::MMWave::Recording
