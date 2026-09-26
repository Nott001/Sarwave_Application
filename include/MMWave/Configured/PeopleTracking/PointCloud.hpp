#pragma once

#include <cstdint>

namespace MMWave::Configured::PeopleTracking {

#pragma pack(push, 1)
    struct PointUnit {
        float elevationUnit;
        float azimuthUnit;
        float dopplerUnit;
        float rangeUnit;
        float snrUnit;
    };

    struct CompressedPoint {
        int8_t  elevation;
        int8_t  azimuth;
        int16_t doppler;
        int16_t range;
        int16_t snr;
    };
#pragma pack(pop)

    enum PointNotAssociated : uint8_t {
        // Point not associated, SNR too weak
        POINT_SNR_WEAK = 253,
        // Point not associated, located outside boundary of interest
        POINT_OUT_OF_BOUNDS = 254,
        //Point not associated, considered as noise
        POINT_LIKELY_NOISE = 255,
    };

    struct PointValue {
        float range;
        float azimuth;
        float elevation;
        float doppler;
        float snr;

        [[nodiscard]] static PointValue Convert(const CompressedPoint& point, const PointUnit& unit) {
            return {
                static_cast<float>(point.range) * unit.rangeUnit,
                static_cast<float>(point.azimuth) * unit.azimuthUnit,
                static_cast<float>(point.elevation) * unit.elevationUnit,
                static_cast<float>(point.doppler) * unit.dopplerUnit,
                static_cast<float>(point.snr) * unit.snrUnit
            };
        }
    };
}
