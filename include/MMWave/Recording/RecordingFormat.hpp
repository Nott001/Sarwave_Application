#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

#include "MMWave/Common/TimeSpan.hpp"

namespace Sarwave::Core::MMWave::Recording {

/// On-disk layout shared by FrameRecorder and FrameRecording -- byte-for-byte the format the C#
/// application writes, so recordings move freely between the two. Everything is little endian.
///
///   File header (12 bytes):   8-byte signature "SWMMWREC", int32 version
///   Then one record per frame:
///     int64  timestamp, in TimeSpan ticks (100 ns)
///     int32  length of the frame in bytes
///     byte[] the frame exactly as it came off the wire (magic word + header + TLVs)
struct RecordingFormat {
    static constexpr std::int32_t Version = 1;
    static constexpr int FileHeaderSize = 12;
    static constexpr int RecordHeaderSize = 12;

    static constexpr std::array<std::uint8_t, 8> Signature{'S', 'W', 'M', 'M', 'W', 'R', 'E', 'C'};

    static void writeFileHeader(std::span<std::uint8_t> destination) {
        std::copy(Signature.begin(), Signature.end(), destination.begin());
        writeLe(destination.subspan(Signature.size()), static_cast<std::uint32_t>(Version), 4);
    }

    static void writeRecordHeader(std::span<std::uint8_t> destination, TimeSpan timestamp,
                                  std::int32_t length) {
        writeLe(destination, static_cast<std::uint64_t>(timestamp.count()), 8);
        writeLe(destination.subspan(8), static_cast<std::uint32_t>(length), 4);
    }

    static void readRecordHeader(std::span<const std::uint8_t> source, TimeSpan& timestamp,
                                 std::int32_t& length) {
        timestamp = TimeSpan(static_cast<std::int64_t>(readLe(source, 8)));
        length =
            static_cast<std::int32_t>(static_cast<std::uint32_t>(readLe(source.subspan(8), 4)));
    }

    static std::int32_t readVersion(std::span<const std::uint8_t> fileHeader) {
        return static_cast<std::int32_t>(
            static_cast<std::uint32_t>(readLe(fileHeader.subspan(Signature.size()), 4)));
    }

   private:
    static void writeLe(std::span<std::uint8_t> to, std::uint64_t value, int bytes) {
        for (int i = 0; i < bytes; ++i)
            to[static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(value >> (8 * i));
    }

    static std::uint64_t readLe(std::span<const std::uint8_t> from, int bytes) {
        std::uint64_t value = 0;
        for (int i = 0; i < bytes; ++i)
            value |= static_cast<std::uint64_t>(from[static_cast<std::size_t>(i)]) << (8 * i);
        return value;
    }
};

}  // namespace Sarwave::Core::MMWave::Recording
