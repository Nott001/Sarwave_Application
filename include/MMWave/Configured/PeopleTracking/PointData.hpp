#pragma once
#include <cstdint>

namespace MMWave::Configured::PeopleTracking {
enum PointNotAssociated : uint8_t {
    // Point not associated, SNR too weak
    POINT_SNR_WEAK = 253,
    // Point not associated, located outside boundary of interest
    POINT_OUT_OF_BOUNDS = 254,
    //Point not associated, considered as noise
    POINT_LIKELY_NOISE = 255,
};
}