#pragma once

#include <cstdint>
#include <cstring>
#include <span>

namespace Sarwave::Core::MMWave::Streaming {

/// The 32 bytes after the magic word: eight little-endian uint32 values, no padding.
struct FrameHeader {
    static constexpr int Size = 32;

    std::uint32_t version;
    std::uint32_t totalPacketLen;
    std::uint32_t platform;
    std::uint32_t frameNumber;
    std::uint32_t timeCpuCycles;
    std::uint32_t numDetectedObj;
    std::uint32_t numTlvs;
    std::uint32_t subFrameNumber;

    /// Reads the header from the first Size bytes of `bytes`. Like the C# ref cast, this takes
    /// the wire's little-endian layout as the host's (true on every supported target).
    static FrameHeader fromBytes(std::span<const std::uint8_t> bytes) {
        FrameHeader header;
        std::memcpy(&header, bytes.data(), Size);
        return header;
    }
};

static_assert(sizeof(FrameHeader) == FrameHeader::Size);

}  // namespace Sarwave::Core::MMWave::Streaming
