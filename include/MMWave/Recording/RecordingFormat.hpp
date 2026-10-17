#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"

namespace Sarwave::Core::MMWave::Recording {

/// On-disk layout shared by FrameRecorder and FrameRecording -- byte-for-byte the format the C#
/// application writes, so recordings move freely between the two. Everything is little endian.
///
///   File header (12 bytes):   8-byte signature "SWMMWREC", int32 version
///   Then one record per frame:
///     int64  timestamp, in TimeSpan ticks (100 ns)
///     int32  length of the frame in bytes
///     byte[] the frame exactly as it came off the wire (magic word + header + TLVs)
///
/// Version 2 adds a second kind of record, for a malformed frame. It has the same 12-byte start,
/// but with the top bit of the length set; the other 31 bits are the length of what follows:
///     byte   where it was noticed: 0 = unspecified, 1 = FrameReader, 2 = EntryPackage
///     int32  the cause of the exception (0 when unspecified)
///     byte   1 if the frame number below is known, otherwise 0
///     uint32 frame number
///     byte[] the exception's detail, UTF-8, to the end of the record
///
/// Version 1 files hold frame records only and are still read.
struct RecordingFormat {
    static constexpr std::int32_t Version = 2;
    static constexpr std::int32_t OldestVersion = 1;
    static constexpr int FileHeaderSize = 12;
    static constexpr int RecordHeaderSize = 12;

    /// Set in a record's length to mark it as a malformed-frame record.
    static constexpr std::uint32_t MalformedFlag = 0x8000'0000u;

    /// Malformed-frame record without its text.
    static constexpr int MalformedFixedSize = 1 + 4 + 1 + 4;

    /// Ceiling for a malformed-frame record, so a corrupt length can't ask for gigabytes.
    static constexpr int MaxMalformedLength = 64 * 1024;

    static constexpr std::uint8_t OriginUnspecified = 0;
    static constexpr std::uint8_t OriginReader = 1;
    static constexpr std::uint8_t OriginEntry = 2;

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

    /// `length` still carries MalformedFlag if the record is one.
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

    static std::vector<std::uint8_t> encodeMalformed(
        const Exceptions::FrameMalformedException& exception) {
        using namespace Exceptions;
        std::uint8_t origin = OriginUnspecified;
        std::int32_t cause = 0;
        if (const auto* reader = dynamic_cast<const FrameReaderMalformedException*>(&exception)) {
            origin = OriginReader;
            cause = static_cast<std::int32_t>(reader->cause());
        } else if (const auto* entry =
                       dynamic_cast<const FrameEntryMalformedException*>(&exception)) {
            origin = OriginEntry;
            cause = static_cast<std::int32_t>(entry->cause());
        }

        const std::string& text = exception.detail();
        const std::size_t textLength =
            std::min<std::size_t>(text.size(), MaxMalformedLength - MalformedFixedSize);
        std::vector<std::uint8_t> payload(MalformedFixedSize + textLength);

        payload[0] = origin;
        writeLe(std::span(payload).subspan(1), static_cast<std::uint32_t>(cause), 4);
        payload[5] = exception.frameNumber() ? 1 : 0;
        writeLe(std::span(payload).subspan(6), exception.frameNumber().value_or(0), 4);
        std::copy_n(text.begin(), textLength, payload.begin() + MalformedFixedSize);
        return payload;
    }

    /// Throws InvalidDataException when the record is too short to be one.
    static std::unique_ptr<Exceptions::FrameMalformedException> decodeMalformed(
        std::span<const std::uint8_t> payload) {
        using namespace Exceptions;
        if (payload.size() < static_cast<std::size_t>(MalformedFixedSize))
            throw InvalidDataException("A malformed-frame record is too short.");

        const auto cause =
            static_cast<std::int32_t>(static_cast<std::uint32_t>(readLe(payload.subspan(1), 4)));
        const std::optional<std::uint32_t> frameNumber =
            payload[5] != 0
                ? std::optional(static_cast<std::uint32_t>(readLe(payload.subspan(6), 4)))
                : std::nullopt;
        std::string detail(payload.begin() + MalformedFixedSize, payload.end());

        switch (payload[0]) {
            case OriginReader:
                return std::make_unique<FrameReaderMalformedException>(
                    static_cast<FrameReaderMalformedException::Reason>(cause), std::move(detail),
                    frameNumber);
            case OriginEntry:
                return std::make_unique<FrameEntryMalformedException>(
                    static_cast<FrameEntryMalformedException::Reason>(cause), std::move(detail),
                    frameNumber);
            default:
                return std::make_unique<FrameMalformedException>(std::move(detail), frameNumber);
        }
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
