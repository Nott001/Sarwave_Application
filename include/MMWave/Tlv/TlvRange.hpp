#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <span>

#include "MMWave/Streaming/Frame.hpp"
#include "MMWave/Streaming/StreamingConstants.hpp"
#include "MMWave/Tlv/TlvEntry.hpp"

namespace Sarwave::Core::MMWave::Tlv {

/// Bounds-checked view over a Frame's TLVs. Like Frame::bytes(), only valid while the owning
/// Frame is alive and not disposed.
///
///   for (const TlvEntry& tlv : TlvRange(frame)) { ... }
///
/// Iteration stops early (without error) when a header or payload would run past the frame.
class TlvRange {
   public:
    static constexpr int StartOffset =
        Streaming::FrameHeader::Size + Streaming::StreamingConstants::MagicWordLength;

    explicit TlvRange(const Streaming::Frame& frame)
        : m_span(frame.bytes()), m_numTlvs(frame.header().numTlvs) {
    }

    class Iterator {
       public:
        using iterator_category = std::input_iterator_tag;
        using value_type = TlvEntry;
        using difference_type = std::ptrdiff_t;
        using pointer = const TlvEntry*;
        using reference = const TlvEntry&;

        Iterator() = default;

        Iterator(std::span<const std::uint8_t> span, std::uint32_t remaining)
            : m_span(span), m_remaining(remaining), m_offset(StartOffset) {
            moveNext();
        }

        reference operator*() const {
            return m_current;
        }

        pointer operator->() const {
            return &m_current;
        }

        Iterator& operator++() {
            moveNext();
            return *this;
        }

        void operator++(int) {
            moveNext();
        }

        friend bool operator==(const Iterator& it, std::default_sentinel_t) {
            return it.m_done;
        }

       private:
        void moveNext() {
            if (m_remaining == 0) {
                m_done = true;
                return;
            }

            // Enough bytes for a full TlvHeader?
            if (m_offset + TlvHeader::Size > m_span.size()) {
                m_remaining = 0;
                m_done = true;
                return;
            }

            const TlvHeader hdr = TlvHeader::fromBytes(m_span.subspan(m_offset, TlvHeader::Size));

            // 64-bit arithmetic: a corrupt length can't wrap around and slip past the check.
            const std::uint64_t payloadEnd =
                static_cast<std::uint64_t>(m_offset) + TlvHeader::Size + hdr.length;
            if (payloadEnd > m_span.size()) {
                m_remaining = 0;
                m_done = true;
                return;
            }

            const std::size_t payloadOffset = m_offset + TlvHeader::Size;
            m_current = TlvEntry{hdr.type, m_span.subspan(payloadOffset, hdr.length)};
            m_done = false;
            m_offset = payloadOffset + hdr.length;
            --m_remaining;
        }

        std::span<const std::uint8_t> m_span;
        std::uint32_t m_remaining = 0;
        std::size_t m_offset = 0;
        TlvEntry m_current;
        bool m_done = true;
    };

    [[nodiscard]] Iterator begin() const {
        return {m_span, m_numTlvs};
    }

    [[nodiscard]] std::default_sentinel_t end() const {
        return {};
    }

   private:
    std::span<const std::uint8_t> m_span;
    std::uint32_t m_numTlvs;
};

}  // namespace Sarwave::Core::MMWave::Tlv
