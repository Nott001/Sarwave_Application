#pragma once

#include <format>

#include "../../Tlv/TlvCore.hpp"
#include "PointCloud.hpp"

namespace MMWave::Configured::PeopleTracking {
#pragma pack(push, 1)
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

        [[nodiscard]] static TargetListRange range(const Tlv::TlvEntry& entry) {
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

        [[nodiscard]] static TargetHeightRange range(const Tlv::TlvEntry& entry) {
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

        [[nodiscard]] static PointCloudRange range(const Tlv::TlvEntry& entry) {
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

        [[nodiscard]] static TargetIndexRange range(const Tlv::TlvEntry& entry) {
            return {entry.payload, entry.length};
        }
    }

    // PresenceIndication TLV (type 1021).
    namespace PresenceIndication {
        // Presence Indication TLV (type 1021) is just a single uint32:
        // 1 = presence detected, 0 = no presence detected.
        [[nodiscard]] static bool presence(const Tlv::TlvEntry& entry) {
            uint32_t value = 0;
            std::memcpy(&value, entry.payload, sizeof(value));
            return value != 0;
        }
    }

}