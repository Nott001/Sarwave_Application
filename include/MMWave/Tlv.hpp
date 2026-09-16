#pragma once
#include <cstdint>
#include <cstring>
#include <cstddef>
#include "Streaming.hpp"

namespace MMWave::Tlv {

    // TLV header + known payload types for the demo output format.
#pragma pack(push, 1)
    struct TlvHeader {
        uint32_t type;
        uint32_t length; // payload length in bytes, NOT including this header
    };

    struct DetectedPoint {
        float x;
        float y;
        float z;
        float velocity;
    };
#pragma pack(pop)

    enum TlvType {
        TLV_DETECTED_POINTS = 1,
        TLV_RANGE_PROFILE = 2,
        TLV_NOISE_PROFILE = 3,
        TLV_AZIMUTH_STATIC_HEATMAP = 4,
        TLV_RANGE_DOPPLER_HEATMAP = 5,
        TLV_STATS = 6,
        TLV_DETECTED_POINTS_SIDE_INFO = 7,
    };

    // A view over one TLV entry's payload -- doesn't own the bytes, just
    // points into the Frame's buffer.
    struct TlvEntry {
        uint32_t type;
        const uint8_t* payload;
        uint32_t length;

        // Iterable range of DetectedPoint. Only meaningful when
        // type == TLV_DETECTED_POINTS -- callers should check `type` before
        // calling points().
        struct PointRange {
            const uint8_t* payload;
            uint32_t length;

            [[nodiscard]] const DetectedPoint* begin() const {
                return reinterpret_cast<const DetectedPoint*>(payload);
            }
            [[nodiscard]] const DetectedPoint* end() const {
                return begin() + (length / sizeof(DetectedPoint));
            }
        };

        [[nodiscard]] PointRange points() const {
            return {payload, length};
        }
    };

    // Custom iterator + range for walking every TLV in a completed Frame:
    //
    //   for (const auto& tlv : Tlv::TlvRange(frame)) {
    //       if (tlv.type == Tlv::TLV_DETECTED_POINTS) {
    //           for (const auto& point : tlv.points()) { ... }
    //       }
    //   }
    //
    // Bounds-checked: if numTLVs or any TLV's declared length doesn't
    // actually fit within the frame's bytes (malformed/truncated data),
    // iteration stops early rather than reading out of bounds.
    class TlvRange {
    public:
        explicit TlvRange(const Streaming::Frame& frame)
            : data_(frame.bytes.data()),
              size_(frame.bytes.size()),
              numTlvs_(frame.header.numTLVs),
              startOffset_(sizeof(Streaming::MAGIC_WORD) + sizeof(Streaming::FrameHeader)) {}

        class Iterator {
        public:
            Iterator(const uint8_t* data, std::size_t size, uint32_t remaining, std::size_t offset)
                : data_(data), size_(size), remaining_(remaining), offset_(offset) {
                validate();
            }

            [[nodiscard]] TlvEntry operator*() const {
                TlvHeader hdr{};
                std::memcpy(&hdr, data_ + offset_, sizeof(TlvHeader));
                return {hdr.type, data_ + offset_ + sizeof(TlvHeader), hdr.length};
            }

            Iterator& operator++() {
                TlvHeader hdr{};
                std::memcpy(&hdr, data_ + offset_, sizeof(TlvHeader));
                offset_ += sizeof(TlvHeader) + hdr.length;
                --remaining_;
                validate();
                return *this;
            }

            [[nodiscard]] bool operator!=(const Iterator& other) const {
                return remaining_ != other.remaining_;
            }

        private:
            // Confirms the TLV at the current offset is fully within
            // bounds before it can be dereferenced. If not, this iterator
            // is treated as "ended" early (remaining_ = 0, matching end()'s
            // sentinel value) instead of risking an out-of-bounds read.
            void validate() {
                if (remaining_ == 0) {
                    return;
                }
                if (offset_ + sizeof(TlvHeader) > size_) {
                    remaining_ = 0;
                    return;
                }

                TlvHeader hdr{};
                std::memcpy(&hdr, data_ + offset_, sizeof(TlvHeader));
                if (offset_ + sizeof(TlvHeader) + hdr.length > size_) {
                    remaining_ = 0;
                }
            }

            const uint8_t* data_;
            std::size_t size_;
            uint32_t remaining_;
            std::size_t offset_;
        };

        [[nodiscard]] Iterator begin() const { return {data_, size_, numTlvs_, startOffset_}; }
        [[nodiscard]] Iterator end() const { return {data_, size_, 0, size_}; }

    private:
        const uint8_t* data_;
        std::size_t size_;
        uint32_t numTlvs_;
        std::size_t startOffset_;
    };

}