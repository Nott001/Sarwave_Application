#pragma once

#include "../../Streaming.hpp"
#include "../../Tlv/TlvTypes.hpp"
#include "../../Tlv/TlvCore.hpp"
#include "../../Tlv/TlvOutput.hpp"

namespace MMWave::Configured::PeopleTracking {
    struct TargetData {
        Tlv::TargetList::TargetListRange tracks = Tlv::TargetList::TargetListRange();
        Tlv::TargetHeight::TargetHeightRange heights = Tlv::TargetHeight::TargetHeightRange();
        std::optional<Tlv::TargetIndex::TargetIndexRange> indexes = std::nullopt;
    };

    struct ObjectData {
        std::optional<Tlv::PointCloud::PointCloudRange> point_cloud = std::nullopt;
        std::optional<TargetData> target = std::nullopt;
        std::optional<bool> presence = std::nullopt;
    };

    static ObjectData unpack(const Streaming::Frame& frame) {
        ObjectData value;

        for (const auto& tlv : Tlv::TlvRange(frame)) {
            switch (tlv.type) {
                case Tlv::TLV_POINT_CLOUD: {
                    value.point_cloud = Tlv::PointCloud::range(tlv);
                    break;
                }
                case Tlv::TLV_TARGET_LIST: {
                    if (!value.target.has_value()) value.target = TargetData();
                    value.target.value().tracks = Tlv::TargetList::range(tlv);
                    break;
                }
                case Tlv::TLV_TARGET_HEIGHT: {
                    if (!value.target.has_value()) value.target = TargetData();
                    value.target.value().heights = Tlv::TargetHeight::range(tlv);
                    break;
                }
                case Tlv::TLV_TARGET_INDEX: {
                    if (!value.target.has_value()) value.target = TargetData();
                    value.target.value().indexes = Tlv::TargetIndex::range(tlv);
                    break;
                }
                case Tlv::TLV_PRESENCE_INDICATION: {
                    value.presence = Tlv::PresenceIndication::presence(tlv);
                    break;
                }
                default:;
            }
        }

        return value;
    }
}