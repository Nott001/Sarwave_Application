#include "MMWave/Recording/FrameRecording.hpp"

#include <algorithm>
#include <array>
#include <fstream>

#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Recording/RecordingFormat.hpp"
#include "MMWave/Streaming/StreamingConstants.hpp"

namespace Sarwave::Core::MMWave::Recording {
namespace {

using Streaming::StreamingConstants;

/// Stream.ReadAtLeast(destination, destination.Length, throwOnEndOfStream: false).
std::size_t readAtLeast(std::istream& stream, std::span<std::uint8_t> destination) {
    stream.read(reinterpret_cast<char*>(destination.data()),
                static_cast<std::streamsize>(destination.size()));
    return static_cast<std::size_t>(stream.gcount());
}

bool tryReadExactly(std::istream& stream, std::span<std::uint8_t> destination) {
    return readAtLeast(stream, destination) == destination.size();
}

}  // namespace

FrameRecording FrameRecording::load(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        const std::string full = std::filesystem::absolute(path).lexically_normal().string();
        if (!std::filesystem::exists(path))
            throw FileNotFoundException("Could not find file '" + full + "'.");
        throw IOException("Access to the path '" + full + "' is denied.");
    }
    return load(stream);
}

FrameRecording FrameRecording::load(std::istream& stream) {
    std::array<std::uint8_t, RecordingFormat::FileHeaderSize> fileHeader{};
    if (!tryReadExactly(stream, fileHeader) ||
        !std::equal(RecordingFormat::Signature.begin(), RecordingFormat::Signature.end(),
                    fileHeader.begin())) {
        throw InvalidDataException("Not an mmWave frame recording.");
    }

    const std::int32_t version = RecordingFormat::readVersion(fileHeader);
    if (version != RecordingFormat::Version) {
        throw InvalidDataException("Unsupported recording version " + std::to_string(version) +
                                   ".");
    }

    std::vector<Streaming::Frame> frames;
    std::vector<TimeSpan> timestamps;
    bool isTruncated = false;
    std::vector<std::uint8_t> buffer(4096);
    std::array<std::uint8_t, RecordingFormat::RecordHeaderSize> recordHeader{};

    while (true) {
        const std::size_t headerBytes = readAtLeast(stream, recordHeader);
        if (headerBytes == 0) break;  // clean end
        if (headerBytes < recordHeader.size()) {
            isTruncated = true;
            break;
        }

        TimeSpan timestamp{};
        std::int32_t length = 0;
        RecordingFormat::readRecordHeader(recordHeader, timestamp, length);
        if (length < StreamingConstants::MinPacketLength ||
            static_cast<std::uint32_t>(length) > StreamingConstants::MaxPacketLength) {
            throw InvalidDataException("Frame " + std::to_string(frames.size()) +
                                       " has an impossible length (" + std::to_string(length) +
                                       ").");
        }

        if (buffer.size() < static_cast<std::size_t>(length))
            buffer.resize(static_cast<std::size_t>(length));
        const std::span<std::uint8_t> bytes(buffer.data(), static_cast<std::size_t>(length));
        if (!tryReadExactly(stream, bytes)) {
            isTruncated = true;
            break;
        }

        if (!std::equal(StreamingConstants::MagicWord.begin(), StreamingConstants::MagicWord.end(),
                        bytes.begin())) {
            throw InvalidDataException("Frame " + std::to_string(frames.size()) +
                                       " does not start with the magic word.");
        }

        frames.push_back(Streaming::Frame::copyFrom(bytes));
        timestamps.push_back(timestamp);
    }

    return FrameRecording(std::move(frames), std::move(timestamps), isTruncated);
}

void FrameRecording::dispose() {
    // Disposed in place, so a second dispose() is a no-op.
    for (auto& frame : m_frames) frame.dispose();
}

}  // namespace Sarwave::Core::MMWave::Recording
