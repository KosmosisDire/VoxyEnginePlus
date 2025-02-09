#pragma once

#include <cmath>
#include <string>
#include <daxa/daxa.hpp>

struct Vector4 {
    float x;
    float y;
    float z;
    float w;

    // Constructors
    constexpr Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    constexpr Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    constexpr Vector4(const Vector4& other) = default;

    // Static factory methods
    static constexpr Vector4 zero() { return Vector4(0.0f, 0.0f, 0.0f, 0.0f); }
    static constexpr Vector4 one() { return Vector4(1.0f, 1.0f, 1.0f, 1.0f); }
    static constexpr Vector4 identity() { return Vector4(0.0f, 0.0f, 0.0f, 1.0f); }

    // Basic operators
    constexpr Vector4 operator+(const Vector4& rhs) const {
        return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
    }

    constexpr Vector4 operator-(const Vector4& rhs) const {
        return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
    }

    constexpr Vector4 operator*(float scalar) const {
        return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    constexpr Vector4 operator/(float scalar) const {
        float inv = 1.0f / scalar;
        return Vector4(x * inv, y * inv, z * inv, w * inv);
    }

    constexpr Vector4& operator+=(const Vector4& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    constexpr Vector4& operator-=(const Vector4& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    constexpr Vector4& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    constexpr Vector4& operator/=(float scalar) {
        float inv = 1.0f / scalar;
        x *= inv;
        y *= inv;
        z *= inv;
        w *= inv;
        return *this;
    }

    constexpr Vector4 operator-() const {
        return Vector4(-x, -y, -z, -w);
    }

    // Comparison operators
    constexpr bool operator==(const Vector4& rhs) const {
        return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
    }

    constexpr bool operator!=(const Vector4& rhs) const {
        return !(*this == rhs);
    }

    // Utility methods
    [[nodiscard]] float magnitude() const {
        return std::sqrt(sqrMagnitude());
    }

    [[nodiscard]] constexpr float sqrMagnitude() const {
        return x * x + y * y + z * z + w * w;
    }

    [[nodiscard]] Vector4 normalized() const {
        float mag = magnitude();
        if (mag > 1e-6f) {
            return *this / mag;
        }
        return Vector4::zero();
    }

    void normalize() {
        float mag = magnitude();
        if (mag > 1e-6f) {
            float inv = 1.0f / mag;
            x *= inv;
            y *= inv;
            z *= inv;
            w *= inv;
        }
    }

    [[nodiscard]] constexpr float dot(const Vector4& rhs) const {
        return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
    }

    [[nodiscard]] static float distance(const Vector4& a, const Vector4& b) {
        return (b - a).magnitude();
    }

    [[nodiscard]] static Vector4 lerp(const Vector4& a, const Vector4& b, float t) {
        t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
        return Vector4(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t,
            a.w + (b.w - a.w) * t
        );
    }

    [[nodiscard]] static Vector4 lerpUnclamped(const Vector4& a, const Vector4& b, float t) {
        return Vector4(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t,
            a.w + (b.w - a.w) * t
        );
    }

    [[nodiscard]] static Vector4 moveTowards(const Vector4& current, const Vector4& target, float maxDistanceDelta) {
        Vector4 toVector = target - current;
        float dist = toVector.magnitude();
        if (dist <= maxDistanceDelta || dist < 1e-6f)
            return target;
        return current + toVector / dist * maxDistanceDelta;
    }

    [[nodiscard]] static Vector4 scale(const Vector4& a, const Vector4& b) {
        return Vector4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
    }

    void scale(const Vector4& scale) {
        x *= scale.x;
        y *= scale.y;
        z *= scale.z;
        w *= scale.w;
    }

    [[nodiscard]] static Vector4 project(const Vector4& vector, const Vector4& onNormal) {
        float sqrMag = onNormal.sqrMagnitude();
        if (sqrMag < 1e-15f)
            return Vector4::zero();
        float dot = vector.dot(onNormal);
        return onNormal * dot / sqrMag;
    }

    // String conversion
    [[nodiscard]] std::string toString() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + 
               std::to_string(z) + ", " + std::to_string(w) + ")";
    }

    // Daxa conversions
    [[nodiscard]] daxa_f32vec4 toDaxa() const {
        return daxa_f32vec4{x, y, z, w};
    }

    static Vector4 fromDaxa(const daxa_f32vec4& v) {
        return Vector4(v.x, v.y, v.z, v.w);
    }
};

// Global operators
inline constexpr Vector4 operator*(float scalar, const Vector4& vector) {
    return vector * scalar;
}