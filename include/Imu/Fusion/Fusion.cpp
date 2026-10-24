#include "Imu/Fusion/Fusion.hpp"

#include <cmath>
#include <cstdio>
#include <numbers>

#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::Core::Imu::Fusion {

using MMWave::cross;
using MMWave::dot;
using MMWave::Matrix4x4;
using MMWave::normalize;

namespace {

/// MathF.Atan2: computed in double and rounded once, which is what .NET does (glibc's atan2f can
/// differ from it in the last bit).
float atan2f(float y, float x) {
    return static_cast<float>(std::atan2(static_cast<double>(y), static_cast<double>(x)));
}

float radiansToDegrees(float radians) {
    // float.RadiansToDegrees: (radians * 180) / π, in that order.
    return (radians * 180.0f) / std::numbers::pi_v<float>;
}

/// "+0.0;-0.0": always signed, one decimal.
std::string signedOneDecimal(float value) {
    char text[32];
    std::snprintf(text, sizeof text, "%+.1f", static_cast<double>(value));
    return text;
}

}  // namespace

Quaternion TiltMath::shortestArc(Vector3 from, Vector3 to) {
    const float d = dot(from, to);
    if (d < -0.999999f) {
        // Opposite vectors: any axis perpendicular to them works. X with its component along
        // them removed turns an upside-down device over about its own left-right axis.
        Vector3 axis = Vector3::unitX() - from * from.x;
        if (axis.lengthSquared() < 1e-6f) axis = Vector3::unitY() - from * from.y;
        return Quaternion::fromAxisAngle(normalize(axis), std::numbers::pi_v<float>);
    }

    const Vector3 c = cross(from, to);
    return normalize(Quaternion{c.x, c.y, c.z, 1 + d});
}

std::string toString(ImuAxis axis) {
    switch (axis) {
        case ImuAxis::PositiveX:
            return "PositiveX";
        case ImuAxis::NegativeX:
            return "NegativeX";
        case ImuAxis::PositiveY:
            return "PositiveY";
        case ImuAxis::NegativeY:
            return "NegativeY";
        case ImuAxis::PositiveZ:
            return "PositiveZ";
        case ImuAxis::NegativeZ:
            return "NegativeZ";
    }
    return std::to_string(static_cast<int>(axis));
}

ImuMounting ImuMounting::fromAxes(ImuAxis radarRight, ImuAxis radarForward, ImuAxis radarUp) {
    // transform(v, m) is v * m, so column i of m picks the radar's component i out of the IMU
    // vector. An IMU axis used twice leaves a zero row, which the determinant check catches.
    Matrix4x4 m = Matrix4x4::identity();
    m.m[0][0] = m.m[1][1] = m.m[2][2] = 0;
    const auto place = [&m](ImuAxis axis, int column) {
        const int row = static_cast<int>(axis) / 2;
        m.m[row][column] = static_cast<int>(axis) % 2 == 0 ? 1.0f : -1.0f;
    };
    place(radarRight, 0);
    place(radarForward, 1);
    place(radarUp, 2);

    if (std::abs(m.determinant3x3() - 1) > 1e-4f) {
        throw ArgumentException("(" + toString(radarRight) + ", " + toString(radarForward) + ", " +
                                toString(radarUp) +
                                ") is not a rotation: the axes must be distinct and right-handed "
                                "(right × forward = up).");
    }

    ImuMounting mounting;
    mounting.rotation = normalize(Quaternion::fromRotationMatrix(m));
    return mounting;
}

ImuMounting ImuMounting::withLevelCalibration(Vector3 measuredUp) const {
    const Quaternion trim = TiltMath::shortestArc(normalize(measuredUp), Vector3::unitZ());
    ImuMounting trimmed = *this;
    trimmed.rotation = normalize(MMWave::concatenate(rotation, trim));
    return trimmed;
}

TiltCorrection TiltCorrection::fromUp(Vector3 up) {
    const float lengthSquared = up.lengthSquared();
    if (!(lengthSquared > 1e-6f) || !std::isfinite(lengthSquared)) return none();

    up = normalize(up);

    // Boresight +Y tilted up by θ sees up = (0, sin θ, cos θ); right side lowered by φ sees
    // up = (−sin φ, 0, cos φ).
    const float pitch = atan2f(up.y, std::sqrt(up.x * up.x + up.z * up.z));
    const float roll = atan2f(-up.x, up.z);

    TiltCorrection tilt;
    tilt.m_rotation = TiltMath::shortestArc(up, Vector3::unitZ());
    tilt.m_pitchDegrees = radiansToDegrees(pitch);
    tilt.m_rollDegrees = radiansToDegrees(roll);
    tilt.m_isValid = true;
    return tilt;
}

std::string TiltCorrection::toString() const {
    if (!m_isValid) return "no tilt data";
    return "pitch " + signedOneDecimal(m_pitchDegrees) + "°, roll " +
           signedOneDecimal(m_rollDegrees) + "°";
}

}  // namespace Sarwave::Core::Imu::Fusion
