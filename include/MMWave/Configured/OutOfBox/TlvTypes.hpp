#pragma once

#include <format>
#include <string_view>
#include <cstdio>

namespace MMWave::Configured::OutOfBox {

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
};

constexpr const char* tlvTypeName(const TlvType type)
{
    switch (type) {
        case TLV_DETECTED_POINTS: return "TLV_DETECTED_POINTS";
        case TLV_RANGE_PROFILE: return "TLV_RANGE_PROFILE";
        case TLV_NOISE_PROFILE: return "TLV_NOISE_PROFILE";
        case TLV_AZIMUTH_STATIC_HEATMAP: return "TLV_AZIMUTH_STATIC_HEATMAP";
        case TLV_RANGE_DOPPLER_HEATMAP: return "TLV_RANGE_DOPPLER_HEATMAP";
        case TLV_STATS: return "TLV_STATS";
        case TLV_DETECTED_POINTS_SIDE_INFO: return "TLV_DETECTED_POINTS_SIDE_INFO";
        default: return nullptr;
    }
}

}

namespace std {

template <>
struct formatter<MMWave::Configured::OutOfBox::TlvType> : formatter<string_view>
{
    template <typename FormatContext>
    auto format(const MMWave::Configured::OutOfBox::TlvType type, FormatContext& ctx) const
    {
        const char* name = MMWave::Configured::OutOfBox::tlvTypeName(type);
        if (name) {
            return formatter<string_view>::format(string_view{name}, ctx);
        }
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(type));
        return formatter<string_view>::format(string_view{buf}, ctx);
    }
};

}
