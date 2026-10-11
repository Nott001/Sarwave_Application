#pragma once

#include <array>
#include <cmath>

/// Stand-ins for System.Numerics.Vector3 / Matrix4x4: plain float aggregates with the handful of
/// operations the C# code uses.
namespace Sarwave::Core::MMWave {

struct Vector3 {
    float x = 0;
    float y = 0;
    float z = 0;

    [[nodiscard]] float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    friend constexpr Vector3 operator+(Vector3 a, Vector3 b) {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    friend constexpr Vector3 operator-(Vector3 a, Vector3 b) {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    friend constexpr Vector3 operator*(Vector3 a, float s) {
        return {a.x * s, a.y * s, a.z * s};
    }

    friend constexpr bool operator==(const Vector3&, const Vector3&) = default;
};

/// Row-major 4x4: m[row][column], i.e. M11 = m[0][0], M12 = m[0][1], ... M44 = m[3][3].
struct Matrix4x4 {
    std::array<std::array<float, 4>, 4> m{};

    friend constexpr bool operator==(const Matrix4x4&, const Matrix4x4&) = default;
};

}  // namespace Sarwave::Core::MMWave
