#pragma once

#include <cstdint>

/// TLVs of the SDK's out-of-box demo.
namespace Sarwave::Core::MMWave::Configured::OutOfBox {

enum class TlvType : std::uint32_t {
    TlvDetectedPoints = 1,
    TlvRangeProfile = 2,
    TlvNoiseProfile = 3,
    TlvAzimuthStaticHeatmap = 4,
    TlvRangeDopplerHeatmap = 5,
    TlvStats = 6,
    TlvDetectedPointsSideInfo = 7,
};

/// One Detected Points TLV record: 4 little-endian floats.
struct TlvDetectedPoint {
    float x;
    float y;
    float z;
    float velocity;
};
static_assert(sizeof(TlvDetectedPoint) == 16);

}  // namespace Sarwave::Core::MMWave::Configured::OutOfBox
