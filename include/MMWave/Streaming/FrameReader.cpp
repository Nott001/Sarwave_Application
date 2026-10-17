#include "MMWave/Streaming/FrameReader.hpp"

#include <algorithm>

#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"
#include "MMWave/Streaming/StreamingConstants.hpp"
#include "MMWave/Tlv/TlvEntry.hpp"

namespace Sarwave::Core::MMWave::Streaming {
namespace {

using SC = StreamingConstants;
using Exceptions::FrameReaderMalformedException;

/// The header behind the magic word at the front of `data`. The caller has already confirmed
/// data.size() >= MinPacketLength.
FrameHeader headerAt(std::span<const std::uint8_t> data) {
    return FrameHeader::fromBytes(data.subspan(SC::MagicWordLength));
}

void removeFront(std::vector<std::uint8_t>& buffer, std::size_t count) {
    buffer.erase(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(count));
}

/// True if the packet's TLVs, as counted by its header, chain from the end of the header to
/// within one padding's length of the end of the packet. Only the lengths are followed; what the
/// TLVs contain is not this reader's business.
///
/// Bytes lost inside a TLV shift every later TLV header, whose length field then reads as
/// garbage, so the chain almost always breaks. It cannot break when the loss is inside the last
/// TLV or the padding: such a packet is returned with the first bytes of the next packet at its
/// end. See borrowedLength for how the next packet is kept.
bool tlvsFit(std::span<const std::uint8_t> packet) {
    std::uint32_t remaining = headerAt(packet).numTlvs;
    std::uint64_t offset = SC::MinPacketLength;

    for (; remaining > 0; remaining--) {
        if (offset + Tlv::TlvHeader::Size > packet.size()) return false;

        const Tlv::TlvHeader header =
            Tlv::TlvHeader::fromBytes(packet.subspan(offset, Tlv::TlvHeader::Size));
        offset += Tlv::TlvHeader::Size + static_cast<std::uint64_t>(header.length);
        if (offset > packet.size()) return false;
    }

    return packet.size() - offset < static_cast<std::uint64_t>(SC::PacketAlignment);
}

/// How many bytes at the end of a complete packet look like the start of the next one's magic
/// word. A packet that lost bytes where tlvsFit cannot tell (its padding, mostly) is made up to
/// its announced length with exactly that many bytes of the next packet. Leaving them in the
/// buffer keeps the next magic word whole, so the next packet is found instead of lost.
///
/// The packet itself is still returned at its full length. If the match was a coincidence, the
/// bytes left behind are a few bytes of junk ahead of the real magic word, which the search
/// drops like any other.
std::size_t borrowedLength(std::span<const std::uint8_t> packet) {
    for (std::size_t length = SC::MagicWordLength - 1; length > 0; length--) {
        if (packet.size() >= length &&
            std::equal(SC::MagicWord.begin(), SC::MagicWord.begin() + length,
                       packet.end() - static_cast<std::ptrdiff_t>(length))) {
            return length;
        }
    }
    return 0;
}

}  // namespace

void FrameReader::reset() {
    m_buffer.clear();
    m_synced = false;
    m_headerParsed = false;
    m_searched = 0;
    m_skippedFrames = 0;
}

std::optional<Frame> FrameReader::readNextFrame(std::stop_token stop) {
    while (true) {
        if (stop.stop_requested()) throw OperationCanceledException();

        Frame frame;
        if (tryExtractFrame(frame)) return frame;

        if (!fillBuffer(stop)) return std::nullopt;
    }
}

void FrameReader::append(std::span<const std::uint8_t> bytes) {
    m_buffer.insert(m_buffer.end(), bytes.begin(), bytes.end());
}

bool FrameReader::fillBuffer(const std::stop_token& stop) {
    if (!m_ctx) return false;  // no port: the stream has ended

    std::size_t n = 0;
    try {
        n = m_ctx->readData(m_staged, stop);
    } catch (const OperationCanceledException&) {
        throw;
    } catch (const TimeoutException&) {
        // No data within the port's timeout: not an error, just loop and re-check the token.
        return true;
    } catch (const std::exception& e) {
        throw IOException(std::string("Data port read error: ") + e.what());
    }

    if (n == 0) return false;

    append(std::span(m_staged).first(n));
    return true;
}

/// Gives up on the packet at the front of the buffer. Only its magic word is removed: the rest
/// stays to be searched, because a packet that arrived short ends with the start of the next
/// one, whose magic word may so far be only partly received.
void FrameReader::skipSyncedPacket() {
    removeFront(m_buffer, SC::MagicWordLength);
    m_synced = false;
    m_headerParsed = false;
    m_searched = 0;
    m_skippedFrames++;
}

/// Same state machine as the C# version (resync on any magic word seen inside a frame, header
/// parsed as soon as it is available, frame complete at TotalPacketLen), run over whatever has
/// arrived so far. On success, the frame's bytes are copied into a new Frame before being
/// removed from the buffer.
bool FrameReader::tryExtractFrame(Frame& frame) {
    while (true) {
        const std::span<const std::uint8_t> data(m_buffer);
        const int length = static_cast<int>(data.size());

        int end;
        if (!m_synced) {
            end = length;
        } else if (m_headerParsed) {
            // TotalPacketLen was validated <= MaxPacketLength when m_headerParsed was set.
            end = std::min(length, static_cast<int>(headerAt(data).totalPacketLen));
        } else {
            end = std::min(length, SC::MinPacketLength);
        }

        const int from = std::max(m_searched - (SC::MagicWordLength - 1), m_synced ? 1 : 0);

        if (end - from >= SC::MagicWordLength) {
            const auto first = data.begin() + from;
            const auto last = data.begin() + end;
            const auto found = std::search(first, last, SC::MagicWord.begin(), SC::MagicWord.end());
            if (found != last) {
                const auto at = static_cast<std::size_t>(found - data.begin());

                // A packet was already under way: it lost at least a magic word's worth of
                // bytes, so the next packet began inside the length it had announced.
                const bool interrupted = m_synced;
                const std::optional<std::uint32_t> interruptedNumber =
                    m_headerParsed ? std::optional(headerAt(data).frameNumber) : std::nullopt;

                if (at > 0) removeFront(m_buffer, at);

                m_synced = true;
                m_headerParsed = false;
                m_searched = SC::MagicWordLength;

                if (interrupted) {
                    m_skippedFrames++;
                    throw FrameReaderMalformedException(
                        FrameReaderMalformedException::Reason::Interrupted,
                        "The next packet began after " + std::to_string(at) + " bytes.",
                        interruptedNumber);
                }

                continue;
            }
        }

        m_searched = end;

        if (!m_synced) {
            // Nothing but garbage so far: keep only a tail that could be the start of a magic
            // word, so the buffer can't grow without bound.
            const int drop = length - (SC::MagicWordLength - 1);
            if (drop > 0) removeFront(m_buffer, static_cast<std::size_t>(drop));

            m_searched = static_cast<int>(m_buffer.size());
            return false;
        }

        if (!m_headerParsed) {
            if (length < SC::MinPacketLength) return false;

            const std::uint32_t total = headerAt(data).totalPacketLen;
            if (total < static_cast<std::uint32_t>(SC::MinPacketLength) ||
                total > SC::MaxPacketLength || total % SC::PacketAlignment != 0) {
                // Corrupt header: discard this sync word and hunt for the next one.
                skipSyncedPacket();
                throw FrameReaderMalformedException(
                    FrameReaderMalformedException::Reason::Header,
                    "The header announces a packet of " + std::to_string(total) + " bytes.");
            }

            m_headerParsed = true;
            continue;  // search window now extends to TotalPacketLen
        }

        const FrameHeader header = headerAt(data);
        const std::uint32_t total = header.totalPacketLen;
        if (static_cast<std::uint32_t>(length) >= total) {
            const auto packet = data.first(total);

            if (!tlvsFit(packet)) {
                skipSyncedPacket();
                throw FrameReaderMalformedException(FrameReaderMalformedException::Reason::TlvChain,
                                                    "Its " + std::to_string(header.numTlvs) +
                                                        " TLVs do not chain to the end of its " +
                                                        std::to_string(total) + " bytes.",
                                                    header.frameNumber);
            }

            frame = Frame::copyFrom(packet);
            removeFront(m_buffer, total - borrowedLength(packet));
            m_synced = false;
            m_headerParsed = false;
            m_searched = 0;
            return true;
        }

        return false;
    }
}

}  // namespace Sarwave::Core::MMWave::Streaming
