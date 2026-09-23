#pragma once
#include <cstdint>
#include <cstring>
#include <cstddef>
#include "../Streaming.hpp"
#include "TlvOutput.hpp"

namespace MMWave::Tlv {

#pragma pack(push, 1)
    struct TlvHeader {
        uint32_t type;
        uint32_t length; // payload length in bytes, NOT including this header
    };
#pragma pack(pop)

    // Custom iterator + range for walking every TLV in a completed Frame:
    //
    //   for (const auto& tlv : Tlv::TlvRange(frame)) {
    //       switch (tlv.type) {
    //           case Tlv::TLV_POINT_CLOUD:
    //               for (const auto& point : tlv.compressedPoints()) { ... }
    //               break;
    //           case Tlv::TLV_TARGET_LIST:
    //               for (const auto& target : tlv.targets()) { ... }
    //               break;
    //           case Tlv::TLV_TARGET_INDEX:
    //               for (uint8_t tid : tlv.targetIndices()) { ... }
    //               break;
    //           case Tlv::TLV_PRESENCE_INDICATION:
    //               bool present = tlv.presence() == 1;
    //               break;
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
                // hdr.type is the raw wire value (may not match any named
                // enumerator, e.g. a TLV type this header hasn't been
                // updated for yet) -- well-defined since TlvType has a
                // fixed uint32_t underlying type.
                return {static_cast<TlvType>(hdr.type), data_ + offset_ + sizeof(TlvHeader), hdr.length};
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