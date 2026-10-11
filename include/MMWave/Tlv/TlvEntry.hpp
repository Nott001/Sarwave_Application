#pragma once

#include <cstdint>
#include <cstring>
#include <span>

namespace Sarwave::Core::MMWave::Tlv {

/// The type/length pair in front of every TLV payload. Read explicitly as little endian, which
/// is what the 3D People Tracking UART format documents.
struct TlvHeader {
    static constexpr int Size = sizeof(std::uint32_t) * 2;

    std::uint32_t type;
    std::uint32_t length;

    static TlvHeader fromBytes(std::span<const std::uint8_t> bytes) {
        const auto le32 = [&](std::size_t at) {
            return static_cast<std::uint32_t>(bytes[at]) |
                   static_cast<std::uint32_t>(bytes[at + 1]) << 8 |
                   static_cast<std::uint32_t>(bytes[at + 2]) << 16 |
                   static_cast<std::uint32_t>(bytes[at + 3]) << 24;
        };
        return {le32(0), le32(4)};
    }
};

/// One TLV: its type and a view of its payload. Only valid while the Frame it came from is.
struct TlvEntry {
    std::uint32_t type = 0;
    std::span<const std::uint8_t> bytes;
};

}  // namespace Sarwave::Core::MMWave::Tlv
