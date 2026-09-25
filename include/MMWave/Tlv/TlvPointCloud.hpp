#pragma once

#include <cstddef>
#include <cstdint>

#include "TlvCore.hpp"

namespace MMWave::Tlv {

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

}
