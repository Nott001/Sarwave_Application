#pragma once

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

enum TargetIndexReserved : uint8_t {
    // Reserved Target Index values (Target Index TLV, type 1011). Any byte
    // value 0-249 is a real track ID; 250-252 are unused/reserved.
    TARGET_INDEX_SNR_TOO_WEAK = 253,        // point not associated: SNR too weak
    TARGET_INDEX_OUTSIDE_BOUNDARY = 254,    // not associated: outside boundary of interest
    TARGET_INDEX_NOISE = 255,               // not associated: considered noise
};

}
