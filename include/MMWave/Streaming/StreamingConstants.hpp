#pragma once

#include <array>
#include <cstdint>

#include "MMWave/Streaming/FrameHeader.hpp"

namespace Sarwave::Core::MMWave::Streaming {

struct StreamingConstants {
    /// This value is used to indicate that the streaming has entered into a new frame.
    static constexpr std::array<std::uint8_t, 8> MagicWord{0x02, 0x01, 0x04, 0x03,
                                                           0x06, 0x05, 0x08, 0x07};

    static constexpr int MagicWordLength = 8;

    /// Magic word + header: the smallest packet that can possibly be valid.
    static constexpr int MinPacketLength = MagicWordLength + FrameHeader::Size;

    /// Sanity ceiling for TotalPacketLen so a corrupt header can't make the reader wait forever
    /// for a gigantic frame. Tune to the largest frame your device config can emit.
    static constexpr std::uint32_t MaxPacketLength = 1u << 20;

    /// The device pads every packet after its last TLV so that TotalPacketLen is a multiple of
    /// this, which also means the padding is always shorter than this.
    static constexpr int PacketAlignment = 32;
};

}  // namespace Sarwave::Core::MMWave::Streaming
