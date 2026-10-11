#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>

#include "MMWave/Streaming/FrameHeader.hpp"

namespace Sarwave::Core::MMWave::Streaming {

/// One complete frame (magic word + header + TLVs), holding its own private copy of those bytes.
///
/// Move-only resource, like the C# struct it ports:
///   - whoever receives a Frame owns it; dispose() (or destruction) releases the bytes;
///   - after dispose(), bytes()/header() throw ObjectDisposedException.
class Frame {
   public:
    /// An empty (disposed) frame.
    Frame() = default;

    Frame(Frame&&) noexcept = default;
    Frame& operator=(Frame&&) noexcept = default;
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;

    /// source is the full frame: magic word + header + TLVs.
    static Frame copyFrom(std::span<const std::uint8_t> source);

    [[nodiscard]] std::span<const std::uint8_t> bytes() const;

    /// The header that follows the magic word.
    [[nodiscard]] FrameHeader header() const;

    [[nodiscard]] bool isDisposed() const {
        return m_data == nullptr;
    }

    void dispose();

   private:
    std::unique_ptr<std::uint8_t[]> m_data;
    std::size_t m_length = 0;
};

}  // namespace Sarwave::Core::MMWave::Streaming
