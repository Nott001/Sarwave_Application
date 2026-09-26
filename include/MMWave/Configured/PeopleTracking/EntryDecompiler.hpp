#pragma once

#include "MMWave/Streaming.hpp"
#include "MMWave/TlvCore.hpp"
#include "TlvTypes.hpp"
#include "TlvOutput.hpp"

namespace MMWave::Configured::PeopleTracking {
    struct TargetData {
        TargetList::TargetListRange tracks = TargetList::TargetListRange();
        TargetHeight::TargetHeightRange heights = TargetHeight::TargetHeightRange();
        std::optional<TargetIndex::TargetIndexRange> indexes = std::nullopt;
    };

    struct ObjectData {
        std::optional<PointCloud::PointCloudRange> point_cloud = std::nullopt;
        std::optional<TargetData> target = std::nullopt;
        std::optional<bool> presence = std::nullopt;
    };

    static ObjectData unpack(const Streaming::Frame& frame) {
        ObjectData value;

        for (const auto& tlv : TlvRange(frame)) {
            switch (tlv.type) {
                case TlvType::TLV_POINT_CLOUD: {
                    value.point_cloud = PointCloud::range(tlv);
                    break;
                }
                case TlvType::TLV_TARGET_LIST: {
                    if (!value.target.has_value()) value.target = TargetData();
                    value.target.value().tracks = TargetList::range(tlv);
                    break;
                }
                case TlvType::TLV_TARGET_HEIGHT: {
                    if (!value.target.has_value()) value.target = TargetData();
                    value.target.value().heights = TargetHeight::range(tlv);
                    break;
                }
                case TlvType::TLV_TARGET_INDEX: {
                    if (!value.target.has_value()) value.target = TargetData();
                    value.target.value().indexes = TargetIndex::range(tlv);
                    break;
                }
                case TlvType::TLV_PRESENCE_INDICATION: {
                    value.presence = PresenceIndication::presence(tlv);
                    break;
                }
                default:;
            }
        }

        return value;
    }
}
