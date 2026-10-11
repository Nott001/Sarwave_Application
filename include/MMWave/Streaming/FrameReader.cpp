#include "MMWave/Streaming/FrameReader.hpp"

#include <algorithm>

#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Streaming/StreamingConstants.hpp"

namespace Sarwave::Core::MMWave::Streaming {
namespace {

using SC = StreamingConstants;

/// TotalPacketLen of the header behind the magic word at the front of `data`. The caller has
/// already confirmed data.size() >= MinPacketLength.
std::uint32_t totalPacketLenAt(const std::vector<std::uint8_t>& data) {
    return FrameHeader::fromBytes(std::span(data).subspan(SC::MagicWordLength)).totalPacketLen;
}

void removeFront(std::vector<std::uint8_t>& buffer, std::size_t count) {
    buffer.erase(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(count));
}

}  // namespace

void FrameReader::reset() {
    m_buffer.clear();
    m_synced = false;
    m_headerParsed = false;
    m_searched = 0;
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

/// Same state machine as the C# version (resync on any magic word seen inside a frame, header
/// parsed as soon as it is available, frame complete at TotalPacketLen), run over whatever has
/// arrived so far. On success, the frame's bytes are copied into a new Frame before being
/// removed from the buffer.
bool FrameReader::tryExtractFrame(Frame& frame) {
    while (true) {
        const auto& data = m_buffer;
        const int length = static_cast<int>(data.size());

        int end;
        if (!m_synced) {
            end = length;
        } else if (m_headerParsed) {
            // TotalPacketLen was validated <= MaxPacketLength when m_headerParsed was set.
            end = std::min(length, static_cast<int>(totalPacketLenAt(data)));
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
                if (at > 0) removeFront(m_buffer, at);

                m_synced = true;
                m_headerParsed = false;
                m_searched = SC::MagicWordLength;
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

            const std::uint32_t total = totalPacketLenAt(data);
            if (total < static_cast<std::uint32_t>(SC::MinPacketLength) ||
                total > SC::MaxPacketLength) {
                // Corrupt header: discard this sync word and hunt for the next one.
                removeFront(m_buffer, SC::MagicWordLength);
                m_synced = false;
                m_searched = 0;
                continue;
            }

            m_headerParsed = true;
            continue;  // search window now extends to TotalPacketLen
        }

        const std::uint32_t total = totalPacketLenAt(data);
        if (static_cast<std::uint32_t>(length) >= total) {
            frame = Frame::copyFrom(std::span(data).first(total));
            removeFront(m_buffer, total);
            m_synced = false;
            m_headerParsed = false;
            m_searched = 0;
            return true;
        }

        return false;
    }
}

}  // namespace Sarwave::Core::MMWave::Streaming
