#pragma once
#include <cstdint>
#include <cstring>
#include <cstddef>
#include "Streaming.hpp"

namespace MMWave::Tlv {

    // TLV header + known payload types for the 3D People Tracking demo output
    // format, per the official "3D People Tracking User's Guide" UART Output
    // Data Format section.
#pragma pack(push, 1)
    struct TlvHeader {
        uint32_t type;
        uint32_t length; // payload length in bytes, NOT including this header
    };

    // Out of Box demo's plain detected-point format: Cartesian XYZ + radial
    // velocity. NOT used by this demo's Point Cloud TLV -- kept only for
    // compatibility if you ever go back to flashing out_of_box_6843_aop.bin.
    struct DetectedPoint {
        float x;
        float y;
        float z;
        float velocity;
    };

    // Point Cloud TLV (type 1020) payload is COMPRESSED, not plain floats.
    // The TLV starts with exactly one PointUnit (the scale factors), followed
    // by an array of CompressedPoint entries. To get real units:
    //   elevation_rad = point.elevation * unit.elevationUnit
    //   azimuth_rad   = point.azimuth   * unit.azimuthUnit
    //   doppler_mps   = point.doppler   * unit.dopplerUnit
    //   range_m       = point.range     * unit.rangeUnit
    //   snr           = point.snr       * unit.snrUnit
    struct PointUnit {
        float elevationUnit;
        float azimuthUnit;
        float dopplerUnit;
        float rangeUnit;
        float snrUnit;
    };

    struct CompressedPoint {
        int8_t  elevation; // radians, needs * PointUnit.elevationUnit
        int8_t  azimuth;   // radians, needs * PointUnit.azimuthUnit
        int16_t doppler;   // m/s,     needs * PointUnit.dopplerUnit
        int16_t range;     // meters,  needs * PointUnit.rangeUnit
        int16_t snr;       // ratio,   needs * PointUnit.snrUnit
    };
    // sizeof(CompressedPoint) == 8 bytes, matching the doc's "each point is
    // defined in 8 bytes."

    // People Tracking demo's per-target record (trackerProc_Target). One of
    // these per currently-tracked object in the TLV_TARGET_LIST payload.
    // Confirmed field-for-field against the official user's guide.
    struct TrackedTarget {
        uint32_t tid;                    // persistent track ID
        float posX, posY, posZ;
        float velX, velY, velZ;
        float accX, accY, accZ;
        float ec[16];                    // 4x4 tracking error covariance matrix
        float g;                         // gating function gain
        float confidenceLevel;
    };
    // sizeof(TrackedTarget) == 112 bytes (4 + 9*4 + 16*4 + 4 + 4).

    // Target Height TLV (type 1012) per-target record.
    struct TargetHeight {
        uint8_t targetID;
        float maxZ;
        float minZ;
    };
#pragma pack(pop)

    enum TlvType {
        // Out of Box demo message types -- not used by this tracking demo's
        // firmware, kept only for reference/compatibility.
        TLV_DETECTED_POINTS = 1,
        TLV_RANGE_PROFILE = 2,
        TLV_NOISE_PROFILE = 3,
        TLV_AZIMUTH_STATIC_HEATMAP = 4,
        TLV_RANGE_DOPPLER_HEATMAP = 5,
        TLV_STATS = 6,
        TLV_DETECTED_POINTS_SIDE_INFO = 7,

        // 3D People Tracking demo message types -- confirmed against the
        // official user's guide's "UART Output Data Format" section.
        TLV_TARGET_LIST = 1010,
        TLV_TARGET_INDEX = 1011,
        TLV_TARGET_HEIGHT = 1012,
        TLV_POINT_CLOUD = 1020,       // compressed, see PointUnit/CompressedPoint
        TLV_PRESENCE_INDICATION = 1021,
    };

    // Reserved Target Index values (Target Index TLV, type 1011). Any byte
    // value 0-249 is a real track ID; 250-252 are unused/reserved.
    enum TargetIndexReserved : uint8_t {
        TARGET_INDEX_SNR_TOO_WEAK = 253,   // point not associated: SNR too weak
        TARGET_INDEX_OUTSIDE_BOUNDARY = 254, // not associated: outside boundary of interest
        TARGET_INDEX_NOISE = 255,          // not associated: considered noise
    };

    // A view over one TLV entry's payload -- doesn't own the bytes, just
    // points into the Frame's buffer.
    struct TlvEntry {
        uint32_t type;
        const uint8_t* payload;
        uint32_t length;

        // Iterable range of the legacy Out of Box DetectedPoint format. Only
        // meaningful when type == TLV_DETECTED_POINTS (i.e. not this demo).
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

        // View over a Point Cloud TLV (type 1020): the leading PointUnit
        // followed by however many CompressedPoint entries fit in the rest
        // of the payload. Only meaningful when type == TLV_POINT_CLOUD.
        //
        //   if (tlv.type == Tlv::TLV_POINT_CLOUD) {
        //       auto pc = tlv.compressedPoints();
        //       const auto& u = pc.unit();
        //       for (const auto& p : pc) {
        //           float rangeM   = p.range     * u.rangeUnit;
        //           float azRad    = p.azimuth   * u.azimuthUnit;
        //           float elRad    = p.elevation * u.elevationUnit;
        //           float dopplerM = p.doppler   * u.dopplerUnit;
        //           float snr      = p.snr       * u.snrUnit;
        //       }
        //   }
        struct CompressedPointRange {
            const uint8_t* payload;
            uint32_t length;

            [[nodiscard]] const PointUnit& unit() const {
                return *reinterpret_cast<const PointUnit*>(payload);
            }
            [[nodiscard]] const CompressedPoint* begin() const {
                return reinterpret_cast<const CompressedPoint*>(payload + sizeof(PointUnit));
            }
            [[nodiscard]] const CompressedPoint* end() const {
                const uint32_t pointsBytes = length > sizeof(PointUnit) ? length - sizeof(PointUnit) : 0;
                return begin() + (pointsBytes / sizeof(CompressedPoint));
            }
        };

        // Iterable range of TrackedTarget. Only meaningful when
        // type == TLV_TARGET_LIST.
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
        // (or a TargetIndexReserved value). Only meaningful when
        // type == TLV_TARGET_INDEX. Correlating this array against the
        // buffered previous-frame point cloud is left to the caller.
        struct TargetIndexRange {
            const uint8_t* payload;
            uint32_t length;

            [[nodiscard]] const uint8_t* begin() const { return payload; }
            [[nodiscard]] const uint8_t* end() const { return payload + length; }
        };

        // Iterable range of TargetHeight. Only meaningful when
        // type == TLV_TARGET_HEIGHT.
        struct TargetHeightRange {
            const uint8_t* payload;
            uint32_t length;

            [[nodiscard]] const TargetHeight* begin() const {
                return reinterpret_cast<const TargetHeight*>(payload);
            }
            [[nodiscard]] const TargetHeight* end() const {
                return begin() + (length / sizeof(TargetHeight));
            }
        };

        [[nodiscard]] PointRange points() const {
            return {payload, length};
        }

        [[nodiscard]] CompressedPointRange compressedPoints() const {
            return {payload, length};
        }

        [[nodiscard]] TargetRange targets() const {
            return {payload, length};
        }

        [[nodiscard]] TargetIndexRange targetIndices() const {
            return {payload, length};
        }

        [[nodiscard]] TargetHeightRange targetHeights() const {
            return {payload, length};
        }

        // Presence Indication TLV (type 1021) is just a single uint32:
        // 1 = presence detected, 0 = no presence detected.
        [[nodiscard]] uint32_t presence() const {
            uint32_t value = 0;
            std::memcpy(&value, payload, sizeof(value));
            return value;
        }
    };

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