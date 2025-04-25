#pragma once

#include "vector3.hpp"
#include <cmath>
#include <string>


struct Quaternion
{
    float x;
    float y;
    float z;
    float w;

    // Constructors
    constexpr Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}

    constexpr Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    constexpr Quaternion(const Quaternion &other) = default;

    // Static factory methods
    static constexpr Quaternion identity() { return Quaternion(0.0f, 0.0f, 0.0f, 1.0f); }

    // Creates a rotation which rotates angle degrees around axis
    static Quaternion angle_axis(float angle, const Vector3 &axis)
    {
        Vector3 normalized_axis = axis.normalized(); // Keep call as Vector3::normalized
        float rad = angle * 0.0174533f; // Convert to radians
        float sinHalfAngle = std::sin(rad * 0.5f);

        return Quaternion(
                   normalized_axis.x * sinHalfAngle,
                   normalized_axis.y * sinHalfAngle,
                   normalized_axis.z * sinHalfAngle,
                   std::cos(rad * 0.5f));
    }

    static Quaternion euler(float x, float y, float z)
    {
        x *= 0.0174533f; // Convert to radians
        y *= 0.0174533f;
        z *= 0.0174533f;

        float cx = std::cos(x * 0.5f);
        float sx = std::sin(x * 0.5f);
        float cy = std::cos(y * 0.5f);
        float sy = std::sin(y * 0.5f);
        float cz = std::cos(z * 0.5f);
        float sz = std::sin(z * 0.5f);

        return Quaternion(
                   sx * cy * cz - cx * sy * sz,
                   cx * sy * cz + sx * cy * sz,
                   cx * cy * sz - sx * sy * cz,
                   cx * cy * cz + sx * sy * sz);
    }

    static Quaternion euler(const Vector3 &euler)
    {
        return Quaternion::euler(euler.x, euler.y, euler.z);
    }

    static Quaternion from_to_rotation(const Vector3 &fromDirection, const Vector3 &toDirection)
    {
        Vector3 from = fromDirection.normalized(); // Keep call
        Vector3 to = toDirection.normalized(); // Keep call

        float dot_val = from.dot(to); // Keep call

        if (dot_val > 0.999999f)
        {
            return Quaternion::identity(); // Keep call
        }

        if (dot_val < -0.999999f)
        {
            Vector3 axis = Vector3(1, 0, 0).cross(from); // Keep call

            if (axis.sqr_magnitude() < 0.000001f) // Update call
            {
                axis = Vector3(0, 1, 0).cross(from); // Keep call
            }

            return angle_axis(180.0f, axis.normalized()); // Update call & Keep Vector3::normalized
        }

        float sqrt_val = std::sqrt((1.0f + dot_val) * 2.0f);
        float invs = 1.0f / sqrt_val;
        Vector3 cross_val = from.cross(to); // Keep call

        return Quaternion(
                   cross_val.x * invs,
                   cross_val.y * invs,
                   cross_val.z * invs,
                   sqrt_val * 0.5f)
               .normalized(); // Keep call
    }

    static Quaternion look_rotation(const Vector3 &forward, Vector3 upwards = Vector3(0, 1, 0))
    {
        Vector3 forward_normalized = forward.normalized(); // Keep call
        Vector3 right_vec = upwards.cross(forward_normalized).normalized(); // Keep calls
        upwards = forward_normalized.cross(right_vec); // Keep call

        float m00 = right_vec.x;
        float m01 = right_vec.y;
        float m02 = right_vec.z;
        float m10 = upwards.x;
        float m11 = upwards.y;
        float m12 = upwards.z;
        float m20 = forward_normalized.x;
        float m21 = forward_normalized.y;
        float m22 = forward_normalized.z;

        float num8 = (m00 + m11) + m22;
        Quaternion quaternion;

        if (num8 > 0.0f)
        {
            float num = std::sqrt(num8 + 1.0f);
            quaternion.w = num * 0.5f;
            num = 0.5f / num;
            quaternion.x = (m12 - m21) * num;
            quaternion.y = (m20 - m02) * num;
            quaternion.z = (m01 - m10) * num;
            return quaternion;
        }

        if ((m00 >= m11) && (m00 >= m22))
        {
            float num7 = std::sqrt(((1.0f + m00) - m11) - m22);
            float num4 = 0.5f / num7;
            quaternion.x = 0.5f * num7;
            quaternion.y = (m01 + m10) * num4;
            quaternion.z = (m02 + m20) * num4;
            quaternion.w = (m12 - m21) * num4;
            return quaternion;
        }

        if (m11 > m22)
        {
            float num6 = std::sqrt(((1.0f + m11) - m00) - m22);
            float num3 = 0.5f / num6;
            quaternion.x = (m10 + m01) * num3;
            quaternion.y = 0.5f * num6;
            quaternion.z = (m21 + m12) * num3;
            quaternion.w = (m20 - m02) * num3;
            return quaternion;
        }

        float num5 = std::sqrt(((1.0f + m22) - m00) - m11);
        float num2 = 0.5f / num5;
        quaternion.x = (m20 + m02) * num2;
        quaternion.y = (m21 + m12) * num2;
        quaternion.z = 0.5f * num5;
        quaternion.w = (m01 - m10) * num2;
        return quaternion;
    }

    // Basic operators
    constexpr Quaternion operator+(const Quaternion &rhs) const
    {
        return Quaternion(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
    }

    constexpr Quaternion operator-(const Quaternion &rhs) const
    {
        return Quaternion(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
    }

    constexpr Quaternion operator*(float scalar) const
    {
        return Quaternion(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    Quaternion operator*(const Quaternion &rhs) const
    {
        return Quaternion(
                   w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
                   w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z,
                   w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x,
                   w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
    }

    Vector3 operator*(const Vector3 &point) const
    {
        Vector3 u(x, y, z);
        float s = w;
        return u * (2.0f * u.dot(point)) +
               point * (s * s - u.dot(u)) +
               u.cross(point) * 2.0f * s;
    }

    constexpr Quaternion operator/(float scalar) const
    {
        float inv = 1.0f / scalar;
        return Quaternion(x * inv, y * inv, z * inv, w * inv);
    }

    constexpr Quaternion operator-() const
    {
        return Quaternion(-x, -y, -z, -w);
    }

    // Comparison operators
    constexpr bool operator==(const Quaternion &rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
    }

    constexpr bool operator!=(const Quaternion &rhs) const
    {
        return !(*this == rhs);
    }

    // Utility methods
    [[nodiscard]] float magnitude() const
    {
        return std::sqrt(sqr_magnitude()); // Update call
    }

    [[nodiscard]] constexpr float sqr_magnitude() const
    {
        return x * x + y * y + z * z + w * w;
    }

    [[nodiscard]] Quaternion normalized() const
    {
        float mag = magnitude();

        if (mag > 1e-6f)
        {
            return *this / mag;
        }

        return Quaternion::identity(); // Keep call
    }

    void normalize()
    {
        float mag = magnitude(); // Keep call

        if (mag > 1e-6f)
        {
            float inv = 1.0f / mag;
            x *= inv;
            y *= inv;
            z *= inv;
            w *= inv;
        }
    }

    [[nodiscard]] constexpr float dot(const Quaternion &rhs) const
    {
        return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
    }

    [[nodiscard]] Vector3 euler_angles() const
    {
        float sinr_cosp = 2.0f * (w * x + y * z);
        float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
        float roll = std::atan2(sinr_cosp, cosr_cosp);

        float sinp = 2.0f * (w * y - z * x);
        float pitch;

        if (std::abs(sinp) >= 1.0f)
        {
            pitch = std::copysign(3.14159f / 2.0f, sinp);
        }
        else
        {
            pitch = std::asin(sinp);

        }

        float siny_cosp = 2.0f * (w * z + x * y);
        float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
        float yaw = std::atan2(siny_cosp, cosy_cosp);

        // Convert to degrees
        return Vector3(
                   roll * 57.2958f,
                   pitch * 57.2958f,
                   yaw * 57.2958f);
    }

    [[nodiscard]] Vector3 get_forward() const
    {
        return *this * Vector3(0, 0, 1);
    }

    [[nodiscard]] Vector3 get_up() const
    {
        return *this * Vector3(0, 1, 0);
    }

    [[nodiscard]] Vector3 get_right() const
    {
        return *this * Vector3(1, 0, 0);
    }

    [[nodiscard]] Quaternion inverse() const
    {
        float n = sqr_magnitude(); // Update call
        return Quaternion(-x / n, -y / n, -z / n, w / n);
    }

    [[nodiscard]] static float angle(const Quaternion &a, const Quaternion &b)
    {
        float dot = std::min(std::abs(a.dot(b)), 1.0f);
        return dot > 1.0f - 1e-6f ? 0.0f : std::acos(dot) * 2.0f * 57.2958f;
    }

    [[nodiscard]] static Quaternion lerp(const Quaternion &a, const Quaternion &b, float t)
    {
        t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
        return Quaternion(
                   a.x + (b.x - a.x) * t,
                   a.y + (b.y - a.y) * t,
                   a.z + (b.z - a.z) * t,
                   a.w + (b.w - a.w) * t)
               .normalized(); // Keep call
    }

    [[nodiscard]] static Quaternion slerp(const Quaternion &a, const Quaternion &b, float t)
    {
        t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);

        float cosHalfTheta = a.dot(b);

        if (cosHalfTheta < 0.0f)

        {
            cosHalfTheta = -cosHalfTheta;
            return slerp(a, -b, t);
        }

        if (cosHalfTheta > 0.999f)
        {
            return lerp(a, b, t);
        }

        float halfTheta = std::acos(cosHalfTheta);
        float sinHalfTheta = std::sqrt(1.0f - cosHalfTheta * cosHalfTheta);

        float ratioA = std::sin((1.0f - t) * halfTheta) / sinHalfTheta;
        float ratioB = std::sin(t * halfTheta) / sinHalfTheta;

        return Quaternion(
                   a.x * ratioA + b.x * ratioB,
                   a.y * ratioA + b.y * ratioB,
                   a.z * ratioA + b.z * ratioB,
                   a.w * ratioA + b.w * ratioB);
    }

    [[nodiscard]] static Quaternion rotate_towards(const Quaternion &from, const Quaternion &to, float maxDegreesDelta)
    {
        float angle_val = Quaternion::angle(from, to); // Keep call

        if (angle_val == 0.0f)
        {
            return to;
        }

        return slerp(from, to, std::min(1.0f, maxDegreesDelta / angle_val)); // Keep call
    }

    // String conversion
    [[nodiscard]] std::string to_string() const
    {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " +
               std::to_string(z) + ", " + std::to_string(w) + ")";
    }
};

// Global operators
inline constexpr Quaternion operator*(float scalar, const Quaternion &quaternion)
{
    return quaternion * scalar;
}
