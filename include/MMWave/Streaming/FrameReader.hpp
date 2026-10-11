#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <stop_token>
#include <vector>

#include "MMWave/Porting/Context.hpp"
#include "MMWave/Streaming/Frame.hpp"

namespace Sarwave::Core::MMWave::Streaming {

/// Reassembles frames from the data port. Bytes accumulate in a private buffer; each Frame
/// returned is its own copy, independent of that buffer, and owned by the caller.
///
/// Usage:
///   while (auto frame = reader.readNextFrame(stop)) handle(*frame);
///
/// Not thread-safe: only one readNextFrame call may be in flight at a time.
class FrameReader {
   public:
    explicit FrameReader(Porting::Context& ctx) : m_ctx(&ctx) {
    }

    /// A reader with no port: feed it with append() and pull frames with tryExtractFrame().
    FrameReader() = default;

    void reset();

    /// Reads and returns the next complete frame, or nullopt if the stream ended.
    /// Throws OperationCanceledException once `stop` is requested, IOException ("Data port
    /// read error: ...") when the port fails.
    std::optional<Frame> readNextFrame(std::stop_token stop = {});

    /// Same state machine, fed from a byte source other than a Context (tests, replays):
    /// appends `bytes` to the buffer.
    void append(std::span<const std::uint8_t> bytes);

    /// The frame-extraction step on its own: true and `frame` set when a complete frame is
    /// buffered. Exposed for feeding the reader from something other than a serial port.
    bool tryExtractFrame(Frame& frame);

    /// Bytes received but not yet returned as part of a frame.
    [[nodiscard]] std::size_t bufferedCount() const {
        return m_buffer.size();
    }

   private:
    bool fillBuffer(const std::stop_token& stop);

    Porting::Context* m_ctx = nullptr;
    std::vector<std::uint8_t> m_buffer;
    std::array<std::uint8_t, 4096> m_staged{};

    bool m_synced = false;        // m_buffer[0..8] is a magic word
    bool m_headerParsed = false;  // the 40 bytes at m_buffer[0] are a validated header
    int m_searched = 0;           // bytes already scanned for a magic word
};

}  // namespace Sarwave::Core::MMWave::Streaming
