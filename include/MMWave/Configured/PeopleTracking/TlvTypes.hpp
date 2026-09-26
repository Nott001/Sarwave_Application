#pragma once

#include <cstdio>
#include <format>
#include <string_view>

namespace MMWave::Configured::PeopleTracking {

// 3D People Tracking demo message types -- confirmed against the
// official user's guide's "UART Output Data Format" section.
enum TlvType : uint32_t {
    TLV_TARGET_LIST = 1010,
    TLV_TARGET_INDEX = 1011,
    TLV_TARGET_HEIGHT = 1012,
    TLV_POINT_CLOUD = 1020,  // compressed, see PointUnit/CompressedPoint
    TLV_PRESENCE_INDICATION = 1021,
};

constexpr const char* tlvTypeName(const TlvType type) {
    switch (type) {
        case TLV_TARGET_LIST:
            return "TLV_TARGET_LIST";
        case TLV_TARGET_INDEX:
            return "TLV_TARGET_INDEX";
        case TLV_TARGET_HEIGHT:
            return "TLV_TARGET_HEIGHT";
        case TLV_POINT_CLOUD:
            return "TLV_POINT_CLOUD";
        case TLV_PRESENCE_INDICATION:
            return "TLV_PRESENCE_INDICATION";
        default:
            return nullptr;
    }
}

}  // namespace MMWave::Configured::PeopleTracking

template <>
struct std::formatter<MMWave::Configured::PeopleTracking::TlvType>
    : std::formatter<std::string_view> {
    template <typename FormatContext>
    auto format(const MMWave::Configured::PeopleTracking::TlvType type, FormatContext& ctx) const {
        const char* name = MMWave::Configured::PeopleTracking::tlvTypeName(type);
        if (name) {
            return std::formatter<std::string_view>::format(std::string_view{name}, ctx);
        }
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(type));
        return std::formatter<std::string_view>::format(std::string_view{buf}, ctx);
    }
};
