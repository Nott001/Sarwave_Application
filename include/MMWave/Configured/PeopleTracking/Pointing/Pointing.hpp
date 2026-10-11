#pragma once

#include <cmath>
#include <cstdint>

#include "MMWave/Common/Numerics.hpp"

/// Point Cloud TLV (1020) records and their decoded forms.
namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Pointing {

/// One compressed point on the wire: 8 bytes, multiplied by the PointUnit to get real values.
struct CompressedPoint {
    std::int8_t elevation;
    std::int8_t azimuth;
    std::int16_t doppler;
    std::int16_t rangeVal;
    std::int16_t snr;
};
static_assert(sizeof(CompressedPoint) == 8);

/// The scale factors at the front of the Point Cloud TLV: 5 little-endian floats.
struct PointUnit {
    float elevationUnit;
    float azimuthUnit;
    float dopplerUnit;
    float rangeUnit;
    float snrUnit;
};
static_assert(sizeof(PointUnit) == 20);

/// Target Index TLV values that mean "this point belongs to no target".
enum class PointNotAssociated : std::uint8_t {
    PointSnrWeak = 253,
    PointOutOfBounds = 254,
    PointLikelyNoise = 255,
};

/// A point in both coordinate systems.
struct PointFigure {
    struct RadialCoordinates {
        float range = 0;
        float azimuth = 0;
        float elevation = 0;
    };

    RadialCoordinates radial;
    Vector3 cartesian;

    float doppler = 0;
    float snr = 0;
};

/// A decompressed point, in radar (spherical) coordinates. Angles in radians.
struct PointValue {
    float range = 0;
    float azimuth = 0;
    float elevation = 0;
    float doppler = 0;
    float snr = 0;

    static PointValue convert(const CompressedPoint& point, const PointUnit& unit) {
        return {
            .range = static_cast<float>(point.rangeVal) * unit.rangeUnit,
            .azimuth = static_cast<float>(point.azimuth) * unit.azimuthUnit,
            .elevation = static_cast<float>(point.elevation) * unit.elevationUnit,
            .doppler = static_cast<float>(point.doppler) * unit.dopplerUnit,
            .snr = static_cast<float>(point.snr) * unit.snrUnit,
        };
    }

    /// Spherical to Cartesian, sensor at the origin: X = lateral (right +), Y = forward, Z = up.
    ///   x = range*cos(elevation)*sin(azimuth)
    ///   y = range*cos(elevation)*cos(azimuth)
    ///   z = range*sin(elevation)
    /// The cos(elevation) factor is what keeps |(x, y, z)| equal to range. All in float.
    [[nodiscard]] Vector3 toVector3() const {
        const float horizontal = range * std::cos(elevation);
        return {horizontal * std::sin(azimuth), horizontal * std::cos(azimuth),
                range * std::sin(elevation)};
    }

    [[nodiscard]] PointFigure toFigure() const {
        return {
            .radial = {range, azimuth, elevation},
            .cartesian = toVector3(),
            .doppler = doppler,
            .snr = snr,
        };
    }
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Pointing
