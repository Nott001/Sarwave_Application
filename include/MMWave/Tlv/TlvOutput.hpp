#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>

#include "TlvCore.hpp"
#include "TlvPointCloud.hpp"

namespace MMWave::Tlv {
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

            [[nodiscard]] uint32_t getCount() const {
                return length / sizeof(DetectedPoint);
            }

            [[nodiscard]] const DetectedPoint* begin() const {
                return reinterpret_cast<const DetectedPoint*>(payload);
            }
            [[nodiscard]] const DetectedPoint* end() const {
                return begin() + (length / sizeof(DetectedPoint));
            }

            [[nodiscard]] const DetectedPoint& operator[](const uint32_t i) const {
                assert(i < getCount());
                return begin()[i];
            }
        };

        [[nodiscard]] static DetectedPointRange range(const TlvEntry& entry) {
            return {entry.payload, entry.length};
        }
    };

    // People Tracking demo's per-target record (trackerProc_Target). One of
    // these per currently-tracked object in the TLV_TARGET_LIST payload.
    // Confirmed field-for-field against the official user's guide.
    struct TargetList {
        uint32_t targetID;               // persistent track ID
        float posX, posY, posZ;
        float velX, velY, velZ;
        float accX, accY, accZ;
        float ec[16];                    // 4x4 tracking error covariance matrix
        float g;                         // gating function gain
        float confidenceLevel;

        // Iterable range of TargetList. Only meaningful when
        // type == TLV_TARGET_LIST.
        struct TargetListRange {
            const uint8_t* payload;
            uint32_t length;

            [[nodiscard]] uint32_t getCount() const {
                return length / sizeof(TargetList);
            }

            [[nodiscard]] const TargetList* begin() const {
                return reinterpret_cast<const TargetList*>(payload);
            }
            [[nodiscard]] const TargetList* end() const {
                return begin() + (length / sizeof(TargetList));
            }

            [[nodiscard]] const TargetList& operator[](const uint32_t i) const {
                assert(i < getCount());
                return begin()[i];
            }
        };

        [[nodiscard]] static TargetListRange range(const TlvEntry& entry) {
            return {entry.payload, entry.length};
        }
    };
    // sizeof(TargetList) == 112 bytes (4 + 9*4 + 16*4 + 4 + 4).

    // Target Height TLV (type 1012) per-target record.
    struct TargetHeight {
        uint32_t targetID;
        float maxZ;
        float minZ;

        // Iterable range of TargetHeight. Only meaningful when
        // type == TLV_TARGET_HEIGHT.
        struct TargetHeightRange {
            const uint8_t* payload;
            uint32_t length;

            [[nodiscard]] uint32_t getCount() const {
                return length / sizeof(TargetHeight);
            }

            [[nodiscard]] const TargetHeight* begin() const {
                return reinterpret_cast<const TargetHeight*>(payload);
            }
            [[nodiscard]] const TargetHeight* end() const {
                return begin() + (length / sizeof(TargetHeight));
            }

            [[nodiscard]] const TargetHeight& operator[](const uint32_t i) const {
                assert(i < getCount());
                return begin()[i];
            }
        };

        [[nodiscard]] static TargetHeightRange range(const TlvEntry& entry) {
            return {entry.payload, entry.length};
        }
    };
#pragma pack(pop)

    // PointCloud TLV (type 1020) wrapper.
    struct PointCloud {
        // PointCloud TLV payload is COMPRESSED, not plain floats.
        // The TLV starts with exactly one PointUnit (the scale factors), followed
        // by an array of CompressedPoint entries.

        struct PointValue {
            float range;
            float azimuth;
            float elevation;
            float doppler;
            float snr;
        };

        [[nodiscard]] static PointValue Convert(const CompressedPoint& point, const PointUnit& unit) {
            return {
                static_cast<float>(point.range) * unit.rangeUnit,
                static_cast<float>(point.azimuth) * unit.azimuthUnit,
                static_cast<float>(point.elevation) * unit.elevationUnit,
                static_cast<float>(point.doppler) * unit.dopplerUnit,
                static_cast<float>(point.snr) * unit.snrUnit
            };
        }

        // View over a Point Cloud TLV (type 1020): the leading PointUnit
        // followed by however many CompressedPoint entries fit in the rest
        // of the payload. Only meaningful when type == TLV_POINT_CLOUD.
        //
        //   auto pc = PointCloud::range(tlv);
        //   const auto& u = pc.unit();
        //   for (const auto& p : pc) {
        //       auto v = PointCloud::Convert(p, u);
        //       float rangeM = v.range;
        //       float azRad  = v.azimuth;
        //       ...
        //   }
        struct PointCloudRange {
            const uint8_t* payload;
            uint32_t length;

            [[nodiscard]] uint32_t getCount() const {
                const uint32_t pointsBytes = length > sizeof(PointUnit) ? length - sizeof(PointUnit) : 0;
                return pointsBytes / sizeof(CompressedPoint);
            }

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

            [[nodiscard]] const CompressedPoint& operator[](const uint32_t i) const {
                assert(i < getCount());
                return begin()[i];
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

            [[nodiscard]] uint32_t getCount() const {
                return length;
            }

            [[nodiscard]] const uint8_t* begin() const { return payload; }
            [[nodiscard]] const uint8_t* end() const { return payload + length; }

            [[nodiscard]] const uint8_t& operator[](const uint32_t i) const {
                assert(i < getCount());
                return begin()[i];
            }
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