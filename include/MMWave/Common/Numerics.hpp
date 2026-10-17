#pragma once

#include <array>
#include <cmath>

/// Stand-ins for System.Numerics.Vector3 / Matrix4x4 / Quaternion: plain float aggregates with
/// the handful of operations the C# code uses, computed the way System.Numerics computes them.
namespace Sarwave::Core::MMWave {

struct Vector3 {
    float x = 0;
    float y = 0;
    float z = 0;

    static constexpr Vector3 zero() {
        return {0, 0, 0};
    }
    static constexpr Vector3 unitX() {
        return {1, 0, 0};
    }
    static constexpr Vector3 unitY() {
        return {0, 1, 0};
    }
    static constexpr Vector3 unitZ() {
        return {0, 0, 1};
    }

    [[nodiscard]] float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    [[nodiscard]] constexpr float lengthSquared() const {
        return x * x + y * y + z * z;
    }

    friend constexpr Vector3 operator+(Vector3 a, Vector3 b) {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    friend constexpr Vector3 operator-(Vector3 a, Vector3 b) {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    friend constexpr Vector3 operator-(Vector3 a) {
        return {-a.x, -a.y, -a.z};
    }

    friend constexpr Vector3 operator*(Vector3 a, float s) {
        return {a.x * s, a.y * s, a.z * s};
    }

    friend constexpr Vector3 operator*(float s, Vector3 a) {
        return a * s;
    }

    /// Component-wise, as Vector3 * Vector3.
    friend constexpr Vector3 operator*(Vector3 a, Vector3 b) {
        return {a.x * b.x, a.y * b.y, a.z * b.z};
    }

    friend constexpr Vector3 operator/(Vector3 a, float s) {
        return {a.x / s, a.y / s, a.z / s};
    }

    Vector3& operator+=(Vector3 b) {
        return *this = *this + b;
    }

    Vector3& operator-=(Vector3 b) {
        return *this = *this - b;
    }

    friend constexpr bool operator==(const Vector3&, const Vector3&) = default;
};

/// Vector3.Dot
constexpr float dot(Vector3 a, Vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/// Vector3.Cross
constexpr Vector3 cross(Vector3 a, Vector3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

/// Vector3.Normalize: NaN components for the zero vector, as in .NET.
inline Vector3 normalize(Vector3 v) {
    return v / v.length();
}

/// Vector3.Distance
inline float distance(Vector3 a, Vector3 b) {
    return (a - b).length();
}

/// Row-major 4x4: m[row][column], i.e. M11 = m[0][0], M12 = m[0][1], ... M44 = m[3][3].
/// As in System.Numerics, vectors are rows: a point transforms as v * M.
struct Matrix4x4 {
    std::array<std::array<float, 4>, 4> m{};

    static constexpr Matrix4x4 identity() {
        Matrix4x4 r;
        for (int i = 0; i < 4; ++i) r.m[i][i] = 1;
        return r;
    }

    /// Determinant of the upper-left 3x3, which is the whole determinant for a matrix whose
    /// last row and column are those of the identity (every rotation used here).
    [[nodiscard]] constexpr float determinant3x3() const {
        return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
               m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
               m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    }

    friend constexpr bool operator==(const Matrix4x4&, const Matrix4x4&) = default;
};

/// System.Numerics.Quaternion: (x, y, z) the vector part, w the scalar part.
struct Quaternion {
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 1;

    static constexpr Quaternion identity() {
        return {0, 0, 0, 1};
    }

    [[nodiscard]] constexpr bool isIdentity() const {
        return x == 0 && y == 0 && z == 0 && w == 1;
    }

    /// Summed as .NET 10's vectorised dot product sums: (x² + y²) + (z² + w²).
    [[nodiscard]] float length() const {
        return std::sqrt((x * x + y * y) + (z * z + w * w));
    }

    [[nodiscard]] constexpr Quaternion conjugate() const {
        return {-x, -y, -z, w};
    }

    /// Quaternion.CreateFromAxisAngle: `axis` must be a unit vector.
    static Quaternion fromAxisAngle(Vector3 axis, float angle) {
        const float half = angle * 0.5f;
        const float s = std::sin(half);
        const float c = std::cos(half);
        return {axis.x * s, axis.y * s, axis.z * s, c};
    }

    /// Quaternion.CreateFromRotationMatrix, for the row-vector matrices System.Numerics uses.
    static Quaternion fromRotationMatrix(const Matrix4x4& matrix) {
        const auto& m = matrix.m;
        const float trace = m[0][0] + m[1][1] + m[2][2];
        Quaternion q;
        if (trace > 0.0f) {
            float s = std::sqrt(trace + 1.0f);
            q.w = s * 0.5f;
            s = 0.5f / s;
            q.x = (m[1][2] - m[2][1]) * s;
            q.y = (m[2][0] - m[0][2]) * s;
            q.z = (m[0][1] - m[1][0]) * s;
        } else if (m[0][0] >= m[1][1] && m[0][0] >= m[2][2]) {
            const float s = std::sqrt(1.0f + m[0][0] - m[1][1] - m[2][2]);
            const float invS = 0.5f / s;
            q.x = 0.5f * s;
            q.y = (m[0][1] + m[1][0]) * invS;
            q.z = (m[0][2] + m[2][0]) * invS;
            q.w = (m[1][2] - m[2][1]) * invS;
        } else if (m[1][1] > m[2][2]) {
            const float s = std::sqrt(1.0f + m[1][1] - m[0][0] - m[2][2]);
            const float invS = 0.5f / s;
            q.x = (m[1][0] + m[0][1]) * invS;
            q.y = 0.5f * s;
            q.z = (m[2][1] + m[1][2]) * invS;
            q.w = (m[2][0] - m[0][2]) * invS;
        } else {
            const float s = std::sqrt(1.0f + m[2][2] - m[0][0] - m[1][1]);
            const float invS = 0.5f / s;
            q.x = (m[2][0] + m[0][2]) * invS;
            q.y = (m[2][1] + m[1][2]) * invS;
            q.z = 0.5f * s;
            q.w = (m[0][1] - m[1][0]) * invS;
        }
        return q;
    }

    friend constexpr bool operator==(const Quaternion&, const Quaternion&) = default;
};

/// Quaternion.Normalize
inline Quaternion normalize(Quaternion q) {
    // As .NET does it: each component divided by the length (not multiplied by its inverse,
    // which rounds differently in the last bit).
    const float length = q.length();
    return {q.x / length, q.y / length, q.z / length, q.w / length};
}

/// Quaternion * Quaternion exactly as .NET 10 computes it (the vectorised DirectXMath
/// XMQuaternionMultiply), so results agree with the C# app to the last bit:
///   result = r·l.w + shuffle(r, 3210)·l.x·(+,−,+,−) + shuffle(r, 2301)·l.y·(+,+,−,−)
///                  + shuffle(r, 1032)·l.z·(−,+,+,−)
constexpr Quaternion multiply(Quaternion l, Quaternion r) {
    return {((r.x * l.w + r.w * l.x) + r.z * l.y) - r.y * l.z,
            ((r.y * l.w - r.z * l.x) + r.w * l.y) + r.x * l.z,
            ((r.z * l.w + r.y * l.x) - r.x * l.y) + r.w * l.z,
            ((r.w * l.w - r.x * l.x) - r.y * l.y) - r.z * l.z};
}

/// Quaternion.Concatenate(first, then): the rotation `first` followed by `then`.
constexpr Quaternion concatenate(Quaternion first, Quaternion then) {
    return multiply(then, first);
}

/// Vector3.Transform(value, rotation): `value` rotated by `rotation`, computed as .NET 10 does
/// (DirectXMath XMVector3Rotate on (value, 1)).
constexpr Vector3 transform(Vector3 v, Quaternion q) {
    const Quaternion temp = concatenate(q.conjugate(), Quaternion{v.x, v.y, v.z, 1.0f});
    const Quaternion result = concatenate(temp, q);
    return {result.x, result.y, result.z};
}

}  // namespace Sarwave::Core::MMWave
