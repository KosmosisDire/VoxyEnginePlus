#pragma once

#include <cmath>
#include <string>
#include <daxa/daxa.hpp>



struct Vector2 {
    float x;
    float y;

    // Constructors
    constexpr Vector2() : x(0.0f), y(0.0f) {}
    constexpr Vector2(float x, float y) : x(x), y(y) {}
    constexpr Vector2(const Vector2& other) = default;

    // Static factory methods
    static constexpr Vector2 zero() { return Vector2(0.0f, 0.0f); }
    static constexpr Vector2 one() { return Vector2(1.0f, 1.0f); }
    static constexpr Vector2 up() { return Vector2(0.0f, 1.0f); }
    static constexpr Vector2 down() { return Vector2(0.0f, -1.0f); }
    static constexpr Vector2 left() { return Vector2(-1.0f, 0.0f); }
    static constexpr Vector2 right() { return Vector2(1.0f, 0.0f); }

    // Basic operators
    constexpr Vector2 operator+(const Vector2& rhs) const {
        return Vector2(x + rhs.x, y + rhs.y);
    }

    constexpr Vector2 operator-(const Vector2& rhs) const {
        return Vector2(x - rhs.x, y - rhs.y);
    }

    constexpr Vector2 operator*(float scalar) const {
        return Vector2(x * scalar, y * scalar);
    }

    constexpr Vector2 operator/(float scalar) const {
        float inv = 1.0f / scalar;
        return Vector2(x * inv, y * inv);
    }

    constexpr Vector2& operator+=(const Vector2& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    constexpr Vector2& operator-=(const Vector2& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    constexpr Vector2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr Vector2& operator/=(float scalar) {
        float inv = 1.0f / scalar;
        x *= inv;
        y *= inv;
        return *this;
    }

    constexpr Vector2 operator-() const {
        return Vector2(-x, -y);
    }

    // Comparison operators
    constexpr bool operator==(const Vector2& rhs) const {
        return x == rhs.x && y == rhs.y;
    }

    constexpr bool operator!=(const Vector2& rhs) const {
        return !(*this == rhs);
    }

    // Utility methods
    [[nodiscard]] float magnitude() const {
        return std::sqrt(sqrMagnitude());
    }

    [[nodiscard]] constexpr float sqrMagnitude() const {
        return x * x + y * y;
    }

    [[nodiscard]] Vector2 normalized() const {
        float mag = magnitude();
        if (mag > 1e-6f) {
            return *this / mag;
        }
        return Vector2::zero();
    }

    void normalize() {
        float mag = magnitude();
        if (mag > 1e-6f) {
            float inv = 1.0f / mag;
            x *= inv;
            y *= inv;
        }
    }

    [[nodiscard]] constexpr float dot(const Vector2& rhs) const {
        return x * rhs.x + y * rhs.y;
    }

    // Returns the signed angle in degrees between from and to
    [[nodiscard]] static float angle(const Vector2& from, const Vector2& to) {
        float denominator = std::sqrt(from.sqrMagnitude() * to.sqrMagnitude());
        if (denominator < 1e-15f)
            return 0.0f;

        float dot = from.dot(to) / denominator;
        // Clamp to handle floating point imprecision
        dot = dot < -1.0f ? -1.0f : (dot > 1.0f ? 1.0f : dot);
        return std::acos(dot) * 57.29578f; // Convert to degrees (180/π)
    }

    [[nodiscard]] static float distance(const Vector2& a, const Vector2& b) {
        return (b - a).magnitude();
    }

    [[nodiscard]] static Vector2 lerp(const Vector2& a, const Vector2& b, float t) {
        t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
        return Vector2(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t
        );
    }

    [[nodiscard]] static Vector2 lerpUnclamped(const Vector2& a, const Vector2& b, float t) {
        return Vector2(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t
        );
    }

    [[nodiscard]] static Vector2 moveTowards(const Vector2& current, const Vector2& target, float maxDistanceDelta) {
        Vector2 toVector = target - current;
        float dist = toVector.magnitude();
        if (dist <= maxDistanceDelta || dist < 1e-6f)
            return target;
        return current + toVector / dist * maxDistanceDelta;
    }

    [[nodiscard]] static Vector2 reflect(const Vector2& inDirection, const Vector2& inNormal) {
        float factor = -2.0f * inDirection.dot(inNormal);
        return Vector2(
            factor * inNormal.x + inDirection.x,
            factor * inNormal.y + inDirection.y
        );
    }

    [[nodiscard]] static Vector2 scale(const Vector2& a, const Vector2& b) {
        return Vector2(a.x * b.x, a.y * b.y);
    }

    void scale(const Vector2& scale) {
        x *= scale.x;
        y *= scale.y;
    }

    // String conversion
    [[nodiscard]] std::string toString() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }

    // Daxa conversions
    [[nodiscard]] daxa_f32vec2 toDaxa() const {
        return daxa_f32vec2{x, y};
    }

    static Vector2 fromDaxa(const daxa_f32vec2& v) {
        return Vector2(v.x, v.y);
    }
};

// Global operators
inline constexpr Vector2 operator*(float scalar, const Vector2& vector) {
    return vector * scalar;
}