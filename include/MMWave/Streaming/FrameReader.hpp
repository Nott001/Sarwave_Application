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
/// The serial link has no checksum and no retransmission, so bytes that go missing on the way
/// simply leave a shorter packet. A packet that no longer holds together -- its header is
/// implausible, its TLVs do not fit, or the next packet starts before it is complete -- is never
/// returned: it is skipped, counted in skippedFrames(), and reported by throwing
/// Exceptions::FrameReaderMalformedException. Skipping gives up only that packet's magic word, so
/// the search for the next one covers the rest of its bytes and the packets after it are
/// unaffected, however many in a row are damaged. The skip is complete before the exception
/// leaves, so the caller only has to call readNextFrame (or tryExtractFrame) again.
///
/// Not thread-safe: only one readNextFrame call may be in flight at a time.
class FrameReader {
   public:
    explicit FrameReader(Porting::Context& ctx) : m_ctx(&ctx) {
    }

    /// A reader with no port: feed it with append() and pull frames with tryExtractFrame().
    FrameReader() = default;

    void reset();

    /// Packets abandoned since the last reset() because they were damaged. A packet that lost
    /// its magic word is never seen at all and is not counted here; it only shows as a gap in
    /// FrameHeader.frameNumber.
    [[nodiscard]] int skippedFrames() const {
        return m_skippedFrames;
    }

    /// Reads and returns the next complete frame, or nullopt if the stream ended.
    /// Throws OperationCanceledException once `stop` is requested, IOException ("Data port
    /// read error: ...") when the port fails, and FrameReaderMalformedException when a damaged
    /// packet was skipped (the reader is then ready for the next call).
    std::optional<Frame> readNextFrame(std::stop_token stop = {});

    /// Same state machine, fed from a byte source other than a Context (tests, replays):
    /// appends `bytes` to the buffer.
    void append(std::span<const std::uint8_t> bytes);

    /// The frame-extraction step on its own: true and `frame` set when a complete frame is
    /// buffered. Exposed for feeding the reader from something other than a serial port.
    /// Throws FrameReaderMalformedException like readNextFrame.
    bool tryExtractFrame(Frame& frame);

    /// Bytes received but not yet returned as part of a frame.
    [[nodiscard]] std::size_t bufferedCount() const {
        return m_buffer.size();
    }

   private:
    bool fillBuffer(const std::stop_token& stop);
    void skipSyncedPacket();

    Porting::Context* m_ctx = nullptr;
    std::vector<std::uint8_t> m_buffer;
    std::array<std::uint8_t, 4096> m_staged{};

    bool m_synced = false;        // m_buffer[0..8] is a magic word
    bool m_headerParsed = false;  // the 40 bytes at m_buffer[0] are a validated header
    int m_searched = 0;           // bytes already scanned for a magic word
    int m_skippedFrames = 0;
};

}  // namespace Sarwave::Core::MMWave::Streaming
