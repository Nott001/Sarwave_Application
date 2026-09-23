#pragma once
#include <cstdint>
#include <cstring>
#include <cstddef>
#include "TlvTypes.hpp"

namespace MMWave::Tlv {

    // A view over one TLV entry's payload -- doesn't own the bytes, just
    // points into the Frame's buffer.
    struct TlvEntry {
        TlvType type;
        const uint8_t* payload;
        uint32_t length;
    };

    // TLV header + known payload types for the 3D People Tracking demo output
    // format, per the official "3D People Tracking User's Guide" UART Output
    // Data Format section.
#pragma pack(push, 1)
    struct DetectedPoint {
        // Out of Box demo's plain detected-point format: Cartesian XYZ + radial
        // velocity. NOT used by this demo's Point Cloud TLV -- kept only for
        // compatibility if you ever go back to flashing out_of_box_6843_aop.bin.
        float x;
        float y;
        float z;
        float velocity;

        // Iterable range of the legacy Out of Box DetectedPoint format. Only
        // meaningful when type == TLV_DETECTED_POINTS (i.e. not this demo).
        struct DetectedPointRange {
            const uint8_t* payload;
            uint32_t length;

            [[nodiscard]] const DetectedPoint* begin() const {
                return reinterpret_cast<const DetectedPoint*>(payload);
            }
            [[nodiscard]] const DetectedPoint* end() const {
                return begin() + (length / sizeof(DetectedPoint));
            }
        };

        [[nodiscard]] static DetectedPointRange range(const TlvEntry& entry) {
            return {entry.payload, entry.length};
        }
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

        // Iterable range of TrackedTarget. Only meaningful when
        // type == TLV_TARGET_LIST.
        struct TrackedTargetRange {
            const uint8_t* payload;
            uint32_t length;

            [[nodiscard]] const TrackedTarget* begin() const {
                return reinterpret_cast<const TrackedTarget*>(payload);
            }
            [[nodiscard]] const TrackedTarget* end() const {
                return begin() + (length / sizeof(TrackedTarget));
            }
        };

        [[nodiscard]] static TrackedTargetRange range(const TlvEntry& entry) {
            return {entry.payload, entry.length};
        }
    };
    // sizeof(TrackedTarget) == 112 bytes (4 + 9*4 + 16*4 + 4 + 4).

    // Target Height TLV (type 1012) per-target record.
    struct TargetHeight {
        uint8_t targetID;
        float maxZ;
        float minZ;

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

        [[nodiscard]] static TargetHeightRange range(const TlvEntry& entry) {
            return {entry.payload, entry.length};
        }
    };
#pragma pack(pop)

    // PointCloud TLV (type 1020) wrapper.
    struct PointCloud {
        // View over a Point Cloud TLV (type 1020): the leading PointUnit
        // followed by however many CompressedPoint entries fit in the rest
        // of the payload. Only meaningful when type == TLV_POINT_CLOUD.
        //
        //   auto pc = PointCloud::range(tlv);
        //   const auto& u = pc.unit();
        //   for (const auto& p : pc) {
        //       float rangeM   = p.range     * u.rangeUnit;
        //       float azRad    = p.azimuth   * u.azimuthUnit;
        //       float elRad    = p.elevation * u.elevationUnit;
        //       float dopplerM = p.doppler   * u.dopplerUnit;
        //       float snr      = p.snr       * u.snrUnit;
        //   }
        struct PointCloudRange {
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

        [[nodiscard]] static PointCloudRange range(const TlvEntry& entry) {
            return {entry.payload, entry.length};
        }
    };

    // TargetIndex TLV (type 1011).
    namespace TargetIndex {

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

        [[nodiscard]] static TargetIndexRange range(const TlvEntry& entry) {
            return {entry.payload, entry.length};
        }
    }

    // PresenceIndication TLV (type 1021).
    namespace PresenceIndication {

        // Presence Indication TLV (type 1021) is just a single uint32:
        // 1 = presence detected, 0 = no presence detected.
        [[nodiscard]] static bool presence(const TlvEntry& entry) {
            uint32_t value = 0;
            std::memcpy(&value, entry.payload, sizeof(value));
            return value != 0;
        }
    }

}