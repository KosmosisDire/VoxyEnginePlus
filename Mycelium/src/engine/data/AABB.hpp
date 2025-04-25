#pragma once

#include <cmath>
#include <string>
#include <daxa/daxa.hpp>
#include "Vector3.hpp" // Assuming Vector3 is in this header

struct AABB
{
    Vector3 min;
    Vector3 max;

    // Constructors
    constexpr AABB() : min(Vector3::zero()), max(Vector3::zero()) {}

    constexpr AABB(const Vector3& min, const Vector3& max) : min(min), max(max) {}

    constexpr AABB(const AABB& other) = default;

    // Static factory methods
    static constexpr AABB Zero() { return AABB(Vector3::zero(), Vector3::zero()); }

    static constexpr AABB Cube(float size)
    {
        Vector3 half = Vector3(size * 0.5f, size * 0.5f, size * 0.5f);
        return AABB(-half, half);
    }

    static constexpr AABB FromCenterSize(const Vector3& center, const Vector3& size)
    {
        Vector3 halfSize = size * 0.5f;
        return AABB(center - halfSize, center + halfSize);
    }

    static constexpr AABB FromExtents(const Vector3& center, const Vector3& extents)
    {
        return AABB(center - extents, center + extents);
    }

    static constexpr AABB FromPoints(const Vector3* points, size_t count)
    {
        if (count == 0) return AABB::Zero();

        Vector3 min = points[0];
        Vector3 max = points[0];

        for (size_t i = 1; i < count; ++i)
        {
            min.x = std::min(min.x, points[i].x);
            min.y = std::min(min.y, points[i].y);
            min.z = std::min(min.z, points[i].z);

            max.x = std::max(max.x, points[i].x);
            max.y = std::max(max.y, points[i].y);
            max.z = std::max(max.z, points[i].z);
        }

        return AABB(min, max);
    }

    // Basic operators
    constexpr AABB operator+(const Vector3& offset) const
    {
        return AABB(min + offset, max + offset);
    }

    constexpr AABB operator-(const Vector3& offset) const
    {
        return AABB(min - offset, max - offset);
    }

    constexpr AABB& operator+=(const Vector3& offset)
    {
        min += offset;
        max += offset;
        return *this;
    }

    constexpr AABB& operator-=(const Vector3& offset)
    {
        min -= offset;
        max -= offset;
        return *this;
    }

    // Comparison operators
    constexpr bool operator==(const AABB& rhs) const
    {
        return min == rhs.min && max == rhs.max;
    }

    constexpr bool operator!=(const AABB& rhs) const
    {
        return !(*this == rhs);
    }

    // Utility methods
    [[nodiscard]] constexpr Vector3 Center() const
    {
        return (min + max) * 0.5f;
    }

    [[nodiscard]] constexpr Vector3 Size() const
    {
        return max - min;
    }

    [[nodiscard]] constexpr Vector3 Extents() const
    {
        return (max - min) * 0.5f;
    }

    [[nodiscard]] constexpr float Volume() const
    {
        Vector3 s = Size();
        return s.x * s.y * s.z;
    }

    [[nodiscard]] constexpr float SurfaceArea() const
    {
        Vector3 s = Size();
        return 2.0f * (s.x * s.y + s.y * s.z + s.z * s.x);
    }

    [[nodiscard]] constexpr bool IsValid() const
    {
        return min.x <= max.x && min.y <= max.y && min.z <= max.z;
    }

    [[nodiscard]] constexpr bool Contains(const Vector3& point) const
    {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    [[nodiscard]] constexpr bool Contains(const AABB& other) const
    {
        return other.min.x >= min.x && other.max.x <= max.x &&
               other.min.y >= min.y && other.max.y <= max.y &&
               other.min.z >= min.z && other.max.z <= max.z;
    }

    [[nodiscard]] constexpr bool Intersects(const AABB& other) const
    {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }

    [[nodiscard]] constexpr AABB Intersection(const AABB& other) const
    {
        Vector3 newMin(
            std::max(min.x, other.min.x),
            std::max(min.y, other.min.y),
            std::max(min.z, other.min.z)
        );

        Vector3 newMax(
            std::min(max.x, other.max.x),
            std::min(max.y, other.max.y),
            std::min(max.z, other.max.z)
        );

        // If no intersection, return an invalid AABB
        if (newMin.x > newMax.x || newMin.y > newMax.y || newMin.z > newMax.z)
        {
            return AABB(Vector3::zero(), Vector3::zero());
        }

        return AABB(newMin, newMax);
    }

    [[nodiscard]] constexpr AABB Expanded(const Vector3& amount) const
    {
        return AABB(min - amount, max + amount);
    }

    [[nodiscard]] constexpr AABB Merged(const AABB& other) const
    {
        Vector3 newMin(
            std::min(min.x, other.min.x),
            std::min(min.y, other.min.y),
            std::min(min.z, other.min.z)
        );

        Vector3 newMax(
            std::max(max.x, other.max.x),
            std::max(max.y, other.max.y),
            std::max(max.z, other.max.z)
        );

        return AABB(newMin, newMax);
    }

    void Encapsulate(const Vector3& point)
    {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);

        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }

    void Encapsulate(const AABB& other)
    {
        min.x = std::min(min.x, other.min.x);
        min.y = std::min(min.y, other.min.y);
        min.z = std::min(min.z, other.min.z);

        max.x = std::max(max.x, other.max.x);
        max.y = std::max(max.y, other.max.y);
        max.z = std::max(max.z, other.max.z);
    }

    [[nodiscard]] Vector3 ClosestPoint(const Vector3& point) const
    {
        return Vector3(
                   std::max(min.x, std::min(point.x, max.x)),
                   std::max(min.y, std::min(point.y, max.y)),
                   std::max(min.z, std::min(point.z, max.z))
               );
    }

    [[nodiscard]] float SqrDistance(const Vector3& point) const
    {
        float sqDist = 0.0f;

        if (point.x < min.x) sqDist += (min.x - point.x) * (min.x - point.x);

        if (point.x > max.x) sqDist += (point.x - max.x) * (point.x - max.x);

        if (point.y < min.y) sqDist += (min.y - point.y) * (min.y - point.y);

        if (point.y > max.y) sqDist += (point.y - max.y) * (point.y - max.y);

        if (point.z < min.z) sqDist += (min.z - point.z) * (min.z - point.z);

        if (point.z > max.z) sqDist += (point.z - max.z) * (point.z - max.z);

        return sqDist;
    }

    [[nodiscard]] float Distance(const Vector3& point) const
    {
        return std::sqrt(SqrDistance(point));
    }

    [[nodiscard]] bool Raycast(const Vector3& origin, const Vector3& direction, float& tMin, float& tMax) const
    {
        float t0x = (min.x - origin.x) / direction.x;
        float t1x = (max.x - origin.x) / direction.x;

        if (t0x > t1x) std::swap(t0x, t1x);

        tMin = t0x;
        tMax = t1x;

        float t0y = (min.y - origin.y) / direction.y;
        float t1y = (max.y - origin.y) / direction.y;

        if (t0y > t1y) std::swap(t0y, t1y);

        tMin = std::max(tMin, t0y);
        tMax = std::min(tMax, t1y);

        float t0z = (min.z - origin.z) / direction.z;
        float t1z = (max.z - origin.z) / direction.z;

        if (t0z > t1z) std::swap(t0z, t1z);

        tMin = std::max(tMin, t0z);
        tMax = std::min(tMax, t1z);

        return tMax >= tMin && tMax > 0.0f;
    }

    // String conversion
    [[nodiscard]] std::string ToString() const
    {
        return "AABB(min: " + min.toString() + ", max: " + max.toString() + ")";
    }

    // Daxa conversions
    struct DaxaAABB
    {
        daxa_f32vec3 min;
        daxa_f32vec3 max;
    };

    [[nodiscard]] DaxaAABB ToDaxa() const
    {
        return DaxaAABB{min.toDaxa(), max.toDaxa()};
    }

    static AABB FromDaxa(const DaxaAABB& aabb)
    {
        return AABB(Vector3::fromDaxa(aabb.min), Vector3::fromDaxa(aabb.max));
    }
};

// Global operators
inline constexpr AABB operator+(const Vector3& offset, const AABB& aabb)
{
    return aabb + offset;
}