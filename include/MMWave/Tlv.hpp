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

    // Out of Box demo's detected-point format: Cartesian XYZ + radial velocity.
    //
    // NOTE: some tracking-oriented demos (3D People Counting/Tracking among
    // them) report points in SPHERICAL form instead --
    //   struct DetectedPointSpherical { float range; float azimuth; float elevation; float velocity; };
    // Whether this binary uses Cartesian or Spherical points is something to
    // confirm against this demo's own output/data-structure guide (or by
    // sanity-checking a few decoded values against known geometry) before
    // trusting DetectedPoint below for TLV_DETECTED_POINTS on this firmware --
    // it has only been verified for the plain Out of Box demo.
    struct DetectedPoint {
        float x;
        float y;
        float z;
        float velocity;
    };

    // People Tracking demo's per-target record (trackerProc_Target in TI's
    // tracker DPU source). One of these per currently-tracked object in the
    // TLV_TARGET_LIST payload.
    struct TrackedTarget {
        uint32_t tid;                    // persistent track ID
        float posX, posY, posZ;
        float velX, velY, velZ;
        float accX, accY, accZ;
        float ec[16];                    // 4x4 tracking error covariance matrix
        float g;                         // gating function gain
        float confidenceLevel;
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

        // These two are specific to the People Tracking demo and are NOT
        // guaranteed to be these exact numeric values -- TI's internal DPU
        // macros (TRACKERPROC_OUTPUT_TARGET_LIST / _INDEX) are not the same
        // thing as the final wire-level MMWDEMO_OUTPUT_EXT_MSG_* type IDs,
        // and those extended IDs vary by SDK/demo version (values as high
        // as 300+ have been seen in other demo variants). Confirm the real
        // values against the source/docs shipped with
        // 3D_people_track_6843_demo.bin before relying on these, e.g. by
        // logging every tlv.type this firmware actually sends and matching
        // it against when a person is known to be in view.
        TLV_TARGET_LIST = 1000,   // placeholder -- verify
        TLV_TARGET_INDEX = 1001,  // placeholder -- verify
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

        // Iterable range of TrackedTarget. Only meaningful when
        // type == TLV_TARGET_LIST -- callers should check `type` before
        // calling targets().
        struct TargetRange {
            const uint8_t* payload;
            uint32_t length;

            [[nodiscard]] const TrackedTarget* begin() const {
                return reinterpret_cast<const TrackedTarget*>(payload);
            }
            [[nodiscard]] const TrackedTarget* end() const {
                return begin() + (length / sizeof(TrackedTarget));
            }
        };

        // Iterable range of raw bytes, one per point in the *previous*
        // frame's point cloud, each giving that point's assigned track ID
        // (reserved high values -- commonly 254/255 -- mean unassociated /
        // out of the boundary box; confirm exact reserved values against
        // this demo's own doc). Only meaningful when
        // type == TLV_TARGET_INDEX. Correlating this array against the
        // buffered previous-frame point cloud is left to the caller.
        struct TargetIndexRange {
            const uint8_t* payload;
            uint32_t length;

            [[nodiscard]] const uint8_t* begin() const { return payload; }
            [[nodiscard]] const uint8_t* end() const { return payload + length; }
        };

        [[nodiscard]] PointRange points() const {
            return {payload, length};
        }

        [[nodiscard]] TargetRange targets() const {
            return {payload, length};
        }

        [[nodiscard]] TargetIndexRange targetIndices() const {
            return {payload, length};
        }
    };

    // Custom iterator + range for walking every TLV in a completed Frame:
    //
    //   for (const auto& tlv : Tlv::TlvRange(frame)) {
    //       switch (tlv.type) {
    //           case Tlv::TLV_DETECTED_POINTS:
    //               for (const auto& point : tlv.points()) { ... }
    //               break;
    //           case Tlv::TLV_TARGET_LIST:
    //               for (const auto& target : tlv.targets()) { ... }
    //               break;
    //           case Tlv::TLV_TARGET_INDEX:
    //               for (uint8_t tid : tlv.targetIndices()) { ... }
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