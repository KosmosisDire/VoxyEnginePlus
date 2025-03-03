#pragma once

#include <cmath>
#include <string>
#include <daxa/daxa.hpp>

struct Vector3 {
    float x;
    float y;
    float z;

    // Constructors
    constexpr Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    constexpr Vector3(const Vector3& other) = default;

    // Static factory methods
    static constexpr Vector3 zero() { return Vector3(0.0f, 0.0f, 0.0f); }
    static constexpr Vector3 one() { return Vector3(1.0f, 1.0f, 1.0f); }
    static constexpr Vector3 up() { return Vector3(0.0f, 1.0f, 0.0f); }
    static constexpr Vector3 down() { return Vector3(0.0f, -1.0f, 0.0f); }
    static constexpr Vector3 left() { return Vector3(-1.0f, 0.0f, 0.0f); }
    static constexpr Vector3 right() { return Vector3(1.0f, 0.0f, 0.0f); }
    static constexpr Vector3 forward() { return Vector3(0.0f, 0.0f, 1.0f); }
    static constexpr Vector3 back() { return Vector3(0.0f, 0.0f, -1.0f); }

    // Basic operators
    constexpr Vector3 operator+(const Vector3& rhs) const {
        return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    constexpr Vector3 operator-(const Vector3& rhs) const {
        return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    constexpr Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    constexpr Vector3 operator/(float scalar) const {
        float inv = 1.0f / scalar;
        return Vector3(x * inv, y * inv, z * inv);
    }

    constexpr Vector3& operator+=(const Vector3& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    constexpr Vector3& operator-=(const Vector3& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    constexpr Vector3& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    constexpr Vector3& operator/=(float scalar) {
        float inv = 1.0f / scalar;
        x *= inv;
        y *= inv;
        z *= inv;
        return *this;
    }

    constexpr Vector3 operator-() const {
        return Vector3(-x, -y, -z);
    }

    // Comparison operators
    constexpr bool operator==(const Vector3& rhs) const {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    constexpr bool operator!=(const Vector3& rhs) const {
        return !(*this == rhs);
    }

    // Utility methods
    [[nodiscard]] float magnitude() const {
        return std::sqrt(sqrMagnitude());
    }

    [[nodiscard]] constexpr float sqrMagnitude() const {
        return x * x + y * y + z * z;
    }

    [[nodiscard]] Vector3 normalized() const {
        float mag = magnitude();
        if (mag > 1e-6f) {
            return *this / mag;
        }
        return Vector3::zero();
    }

    void normalize() {
        float mag = magnitude();
        if (mag > 1e-6f) {
            float inv = 1.0f / mag;
            x *= inv;
            y *= inv;
            z *= inv;
        }
    }

    [[nodiscard]] constexpr float dot(const Vector3& rhs) const {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }

    [[nodiscard]] constexpr Vector3 cross(const Vector3& rhs) const {
        return Vector3(
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x
        );
    }

    [[nodiscard]] static float angle(const Vector3& from, const Vector3& to) {
        float denominator = std::sqrt(from.sqrMagnitude() * to.sqrMagnitude());
        if (denominator < 1e-15f)
            return 0.0f;

        float dot = from.dot(to) / denominator;
        dot = dot < -1.0f ? -1.0f : (dot > 1.0f ? 1.0f : dot);
        return std::acos(dot) * 57.29578f; // Convert to degrees
    }

    [[nodiscard]] static float signedAngle(const Vector3& from, const Vector3& to, const Vector3& axis) {
        float unsignedAngle = angle(from, to);
        float sign = from.cross(to).dot(axis) < 0.0f ? -1.0f : 1.0f;
        return unsignedAngle * sign;
    }

    [[nodiscard]] static float distance(const Vector3& a, const Vector3& b) {
        return (b - a).magnitude();
    }

    [[nodiscard]] static Vector3 lerp(const Vector3& a, const Vector3& b, float t) {
        t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
        return Vector3(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t
        );
    }

    [[nodiscard]] static Vector3 lerpUnclamped(const Vector3& a, const Vector3& b, float t) {
        return Vector3(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t
        );
    }

    [[nodiscard]] static Vector3 moveTowards(const Vector3& current, const Vector3& target, float maxDistanceDelta) {
        Vector3 toVector = target - current;
        float dist = toVector.magnitude();
        if (dist <= maxDistanceDelta || dist < 1e-6f)
            return target;
        return current + toVector / dist * maxDistanceDelta;
    }

    [[nodiscard]] static Vector3 reflect(const Vector3& inDirection, const Vector3& inNormal) {
        float factor = -2.0f * inDirection.dot(inNormal);
        return Vector3(
            factor * inNormal.x + inDirection.x,
            factor * inNormal.y + inDirection.y,
            factor * inNormal.z + inDirection.z
        );
    }

    [[nodiscard]] static Vector3 project(const Vector3& vector, const Vector3& onNormal) {
        float sqrMag = onNormal.sqrMagnitude();
        if (sqrMag < 1e-15f)
            return Vector3::zero();
        float dot = vector.dot(onNormal);
        return onNormal * dot / sqrMag;
    }

    [[nodiscard]] static Vector3 projectOnPlane(const Vector3& vector, const Vector3& planeNormal) {
        return vector - project(vector, planeNormal);
    }

    [[nodiscard]] static Vector3 scale(const Vector3& a, const Vector3& b) {
        return Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
    }

    void scale(const Vector3& scale) {
        x *= scale.x;
        y *= scale.y;
        z *= scale.z;
    }

    // String conversion
    [[nodiscard]] std::string toString() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }

    // Daxa conversions
    [[nodiscard]] daxa_f32vec3 toDaxa() const {
        return daxa_f32vec3{x, y, z};
    }

    static Vector3 fromDaxa(const daxa_f32vec3& v) {
        return Vector3(v.x, v.y, v.z);
    }
};

// Global operators
inline constexpr Vector3 operator*(float scalar, const Vector3& vector) {
    return vector * scalar;
}