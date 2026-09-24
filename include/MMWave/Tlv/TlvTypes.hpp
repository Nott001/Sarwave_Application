#pragma once

#include <format>
#include <string_view>
#include <cstdio>

namespace MMWave::Tlv {

enum TlvType : uint32_t {
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
    TLV_POINT_CLOUD = 1020,                 // compressed, see PointUnit/CompressedPoint
    TLV_PRESENCE_INDICATION = 1021,
};

constexpr const char* tlvTypeName(TlvType type)
{
    switch (type) {
        case TLV_DETECTED_POINTS: return "TLV_DETECTED_POINTS";
        case TLV_RANGE_PROFILE: return "TLV_RANGE_PROFILE";
        case TLV_NOISE_PROFILE: return "TLV_NOISE_PROFILE";
        case TLV_AZIMUTH_STATIC_HEATMAP: return "TLV_AZIMUTH_STATIC_HEATMAP";
        case TLV_RANGE_DOPPLER_HEATMAP: return "TLV_RANGE_DOPPLER_HEATMAP";
        case TLV_STATS: return "TLV_STATS";
        case TLV_DETECTED_POINTS_SIDE_INFO: return "TLV_DETECTED_POINTS_SIDE_INFO";
        case TLV_TARGET_LIST: return "TLV_TARGET_LIST";
        case TLV_TARGET_INDEX: return "TLV_TARGET_INDEX";
        case TLV_TARGET_HEIGHT: return "TLV_TARGET_HEIGHT";
        case TLV_POINT_CLOUD: return "TLV_POINT_CLOUD";
        case TLV_PRESENCE_INDICATION: return "TLV_PRESENCE_INDICATION";
        default: return nullptr;
    }
}

}  // namespace MMWave::Tlv

namespace std {

template <>
struct formatter<MMWave::Tlv::TlvType> : formatter<string_view>
{
    template <typename FormatContext>
    auto format(MMWave::Tlv::TlvType type, FormatContext& ctx) const
    {
        const char* name = MMWave::Tlv::tlvTypeName(type);
        if (name) {
            return formatter<string_view>::format(string_view{name}, ctx);
        }
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(type));
        return formatter<string_view>::format(string_view{buf}, ctx);
    }
};

}  // namespace std
