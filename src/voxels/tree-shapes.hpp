#pragma once

#include <cmath>
#include <random>
#include <algorithm>

// 3D Vector class
struct Vec3
{
    int32_t x, y, z;

    Vec3() : x(0), y(0), z(0) {}

    Vec3(int32_t x, int32_t y, int32_t z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& other) const
    {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other) const
    {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator<<(int shift) const
    {
        return Vec3(x << shift, y << shift, z << shift);
    }

    Vec3 operator>>(int shift) const
    {
        return Vec3(x >> shift, y >> shift, z >> shift);
    }

    Vec3 operator&(int mask) const
    {
        return Vec3(x & mask, y & mask, z & mask);
    }

    Vec3 operator*(int scale) const
    {
        return Vec3(x * scale, y * scale, z * scale);
    }

    float lengthSquared() const
    {
        return static_cast<float>(x*x + y*y + z*z);
    }
};

// Shape interface for defining voxelized geometries
class Shape
{
    public:
        virtual ~Shape() = default;

        // Test if shape intersects cube at position with given scale
        virtual bool Intersects(const Vec3& pos, int scale) const = 0;

        // Test if shape completely contains cube at position with given scale
        virtual bool ContainsCompletely(const Vec3& pos, int scale) const = 0;

        // Test if point is inside shape
        virtual bool Contains(const Vec3& point) const = 0;
};


// Sphere shape implementation
class Sphere : public Shape
{
    private:
        Vec3 center;
        float radius;
        float radiusSquared;

    public:
        Sphere(const Vec3& center, float radius)
            : center(center), radius(radius), radiusSquared(radius * radius)
        {
        }

        bool Intersects(const Vec3& pos, int scale) const override
        {
            // Calculate the minimum distance from the cube to the sphere center
            // by finding the closest point on the cube to the sphere center

            // Cube corners
            Vec3 min = pos;
            Vec3 max = Vec3(pos.x + scale, pos.y + scale, pos.z + scale);

            // Find closest point on cube to sphere center
            Vec3 closest;

            // For each dimension, clamp center to cube bounds
            closest.x = std::max(min.x, std::min(center.x, max.x));
            closest.y = std::max(min.y, std::min(center.y, max.y));
            closest.z = std::max(min.z, std::min(center.z, max.z));

            // Calculate squared distance
            Vec3 diff = closest - center;
            float distSquared = diff.lengthSquared();

            // Cube intersects sphere if closest point is within radius
            return distSquared <= radiusSquared;
        }

        bool ContainsCompletely(const Vec3& pos, int scale) const override
        {
            // A cube is completely contained if all its corners are inside the sphere

            // Check all 8 corners
            Vec3 corners[8] =
            {
                Vec3(pos.x, pos.y, pos.z),
                Vec3(pos.x + scale, pos.y, pos.z),
                Vec3(pos.x, pos.y + scale, pos.z),
                Vec3(pos.x + scale, pos.y + scale, pos.z),
                Vec3(pos.x, pos.y, pos.z + scale),
                Vec3(pos.x + scale, pos.y, pos.z + scale),
                Vec3(pos.x, pos.y + scale, pos.z + scale),
                Vec3(pos.x + scale, pos.y + scale, pos.z + scale)
            };

            // All corners must be inside sphere
            for (const auto& corner : corners)
            {
                Vec3 diff = corner - center;

                if (diff.lengthSquared() > radiusSquared)
                {
                    return false;
                }
            }

            return true;
        }

        bool Contains(const Vec3& point) const override
        {
            Vec3 diff = point - center;
            return diff.lengthSquared() <= radiusSquared;
        }
};

// Cube shape implementation
class Cube : public Shape
{
    private:
        Vec3 min;
        Vec3 max;

    public:
        Cube(const Vec3& center, int size)
        {
            // Calculate min and max from center and size
            int halfSize = size / 2;
            min = Vec3(center.x - halfSize, center.y - halfSize, center.z - halfSize);
            max = Vec3(center.x + halfSize, center.y + halfSize, center.z + halfSize);
        }

        Cube(const Vec3& min, const Vec3& max)
            : min(min), max(max)
        {
        }

        bool Intersects(const Vec3& pos, int scale) const override
        {
            // Two cubes intersect if they overlap in all dimensions
            Vec3 cubeMax(pos.x + scale, pos.y + scale, pos.z + scale);

            return !(cubeMax.x <= min.x || pos.x >= max.x ||
                     cubeMax.y <= min.y || pos.y >= max.y ||
                     cubeMax.z <= min.z || pos.z >= max.z);
        }

        bool ContainsCompletely(const Vec3& pos, int scale) const override
        {
            // Cube contains another cube if all corners of the other cube are inside
            Vec3 cubeMax(pos.x + scale, pos.y + scale, pos.z + scale);

            return (pos.x >= min.x && cubeMax.x <= max.x &&
                    pos.y >= min.y && cubeMax.y <= max.y &&
                    pos.z >= min.z && cubeMax.z <= max.z);
        }

        bool Contains(const Vec3& point) const override
        {
            return (point.x >= min.x && point.x < max.x &&
                    point.y >= min.y && point.y < max.y &&
                    point.z >= min.z && point.z < max.z);
        }
};

// Perlin noise implementation
class Perlin : public Shape
{
    private:
        float threshold;
        int seed;
        float scale;
        int octaves;

        // Permutation table for Perlin noise
        unsigned char perm[512];

        // Helper functions for Perlin noise
        static float Fade(float t)
        {
            return t * t * t * (t * (t * 6 - 15) + 10);
        }

        static float Lerp(float t, float a, float b)
        {
            return a + t * (b - a);
        }

        static float Grad(int hash, float x, float y, float z)
        {
            // Convert low 4 bits of hash code into 12 gradient directions
            int h = hash & 15;
            float u = h < 8 ? x : y;
            float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
            return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
        }

        void InitPermutationTable()
        {
            std::mt19937 rng(seed);
            std::uniform_int_distribution<int> dist(0, 255);

            // Fill permutation table with randomized values 0-255
            for (int i = 0; i < 256; i++)
            {
                perm[i] = static_cast<unsigned char>(dist(rng));
            }

            // Duplicate the permutation table to avoid overflow checks
            for (int i = 0; i < 256; i++)
            {
                perm[i + 256] = perm[i];
            }
        }

        float Noise(float x, float y, float z) const
        {
            // Find unit cube that contains point
            int X = static_cast<int>(std::floor(x)) & 255;
            int Y = static_cast<int>(std::floor(y)) & 255;
            int Z = static_cast<int>(std::floor(z)) & 255;

            // Find relative x, y, z of point in cube
            x -= std::floor(x);
            y -= std::floor(y);
            z -= std::floor(z);

            // Compute fade curves for each of x, y, z
            float u = Fade(x);
            float v = Fade(y);
            float w = Fade(z);

            // Hash coordinates of the 8 cube corners
            int A = perm[X] + Y;
            int AA = perm[A] + Z;
            int AB = perm[A + 1] + Z;
            int B = perm[X + 1] + Y;
            int BA = perm[B] + Z;
            int BB = perm[B + 1] + Z;

            // Add blended results from 8 corners of cube
            float res = Lerp(w, Lerp(v, Lerp(u, Grad(perm[AA], x, y, z),
                                             Grad(perm[BA], x - 1, y, z)),
                                     Lerp(u, Grad(perm[AB], x, y - 1, z),
                                          Grad(perm[BB], x - 1, y - 1, z))),
                             Lerp(v, Lerp(u, Grad(perm[AA + 1], x, y, z - 1),
                                          Grad(perm[BA + 1], x - 1, y, z - 1)),
                                  Lerp(u, Grad(perm[AB + 1], x, y - 1, z - 1),
                                       Grad(perm[BB + 1], x - 1, y - 1, z - 1))));

            // Scale to [-1, 1]
            return res;
        }

        float FractalNoise(float x, float y, float z) const
        {
            float result = 0.0f;
            float amplitude = 1.0f;
            float frequency = 1.0f;
            float max_value = 0.0f;

            // Sum octaves of noise
            for (int i = 0; i < octaves; i++)
            {
                result += amplitude * Noise(x * frequency / scale,
                                            y * frequency / scale,
                                            z * frequency / scale);
                max_value += amplitude;
                amplitude *= 0.5f;
                frequency *= 2.0f;
            }

            // Normalize the result
            return (result / max_value + 1.0f) * 0.5f;  // Scale to [0, 1]
        }

    public:
        Perlin(float threshold = 0.5f, int seed = 42, float scale = 50.0f, int octaves = 4)
            : threshold(threshold), seed(seed), scale(scale), octaves(octaves)
        {
            InitPermutationTable();
        }

        bool Intersects(const Vec3& pos, int scale) const override
        {
            // We need to check if any point in the cube could be above the threshold
            // For Perlin noise, this is hard to determine exactly, so we'll use a heuristic

            // Check corners and center of the cube
            Vec3 corners[9] =
            {
                Vec3(pos.x, pos.y, pos.z),
                Vec3(pos.x + scale, pos.y, pos.z),
                Vec3(pos.x, pos.y + scale, pos.z),
                Vec3(pos.x + scale, pos.y + scale, pos.z),
                Vec3(pos.x, pos.y, pos.z + scale),
                Vec3(pos.x + scale, pos.y, pos.z + scale),
                Vec3(pos.x, pos.y + scale, pos.z + scale),
                Vec3(pos.x + scale, pos.y + scale, pos.z + scale),
                Vec3(pos.x + scale/2, pos.y + scale/2, pos.z + scale/2)  // Center
            };

            for (const auto& point : corners)
            {
                float noise = FractalNoise(static_cast<float>(point.x),
                                           static_cast<float>(point.y),
                                           static_cast<float>(point.z));

                if (noise >= threshold)
                {
                    return true;
                }
            }

            // Additional check with reduced threshold for border cases
            // This helps catch areas that might have high values between sample points
            float relaxedThreshold = threshold * 0.9f;

            for (const auto& point : corners)
            {
                float noise = FractalNoise(static_cast<float>(point.x),
                                           static_cast<float>(point.y),
                                           static_cast<float>(point.z));

                if (noise >= relaxedThreshold)
                {
                    return true;
                }
            }

            return false;
        }

        bool ContainsCompletely(const Vec3& pos, int scale) const override
        {
            // For Perlin noise, this is very difficult to determine exactly
            // We'll check several points and only return true if all are inside

            // Number of samples in each dimension
            const int samplesPerDim = 3;
            const float step = static_cast<float>(scale) / (samplesPerDim - 1);

            // Check grid of points throughout the cube
            for (int x = 0; x < samplesPerDim; x++)
            {
                for (int y = 0; y < samplesPerDim; y++)
                {
                    for (int z = 0; z < samplesPerDim; z++)
                    {
                        float px = pos.x + x * step;
                        float py = pos.y + y * step;
                        float pz = pos.z + z * step;

                        float noise = FractalNoise(px, py, pz);

                        if (noise < threshold)
                        {
                            return false;
                        }
                    }
                }
            }

            return true;
        }

        bool Contains(const Vec3& point) const override
        {
            float noise = FractalNoise(static_cast<float>(point.x),
                                       static_cast<float>(point.y),
                                       static_cast<float>(point.z));

            return noise >= threshold;
        }
};


// Enum for boolean operations
enum class BooleanOperation
{
    UNION,        // Points in either shape
    INTERSECTION, // Points in both shapes
    DIFFERENCE    // Points in first shape but not in second
};

// Composite shape implementation
class CompositeShape : public Shape
{
    private:
        std::vector<const Shape*> shapes;
        BooleanOperation operation;

    public:
        // Constructor for two shapes
        CompositeShape(const Shape* shape1, const Shape* shape2, BooleanOperation op)
            : operation(op)
        {
            shapes.push_back(shape1);
            shapes.push_back(shape2);
        }

        // Constructor for multiple shapes
        CompositeShape(const std::vector<const Shape*>& shapeList, BooleanOperation op)
            : shapes(shapeList), operation(op)
        {
        }

        bool Contains(const Vec3& point) const override
        {
            // Empty shape list
            if (shapes.empty())
            {
                return false;
            }

            // Single shape optimization
            if (shapes.size() == 1)
            {
                return shapes[0]->Contains(point);
            }

            switch (operation)
            {
                case BooleanOperation::UNION:

                    // Point is in at least one shape
                    for (const auto& shape : shapes)
                    {
                        if (shape->Contains(point))
                        {
                            return true;
                        }
                    }

                    return false;

                case BooleanOperation::INTERSECTION:

                    // Point is in all shapes
                    for (const auto& shape : shapes)
                    {
                        if (!shape->Contains(point))
                        {
                            return false;
                        }
                    }

                    return true;

                case BooleanOperation::DIFFERENCE:

                    // Point is in first shape but not in any other
                    if (!shapes[0]->Contains(point))
                    {
                        return false;
                    }

                    // Check that point is not in any of the other shapes
                    for (size_t i = 1; i < shapes.size(); i++)
                    {
                        if (shapes[i]->Contains(point))
                        {
                            return false;
                        }
                    }

                    return true;

                default:
                    return false;
            }
        }

        bool Intersects(const Vec3& pos, int scale) const override
        {
            // Empty shape list
            if (shapes.empty())
            {
                return false;
            }

            // Single shape optimization
            if (shapes.size() == 1)
            {
                return shapes[0]->Intersects(pos, scale);
            }

            // Define corners array outside switch statement for use in multiple cases
            Vec3 corners[8] =
            {
                Vec3(pos.x, pos.y, pos.z),
                Vec3(pos.x + scale, pos.y, pos.z),
                Vec3(pos.x, pos.y + scale, pos.z),
                Vec3(pos.x + scale, pos.y + scale, pos.z),
                Vec3(pos.x, pos.y, pos.z + scale),
                Vec3(pos.x + scale, pos.y, pos.z + scale),
                Vec3(pos.x, pos.y + scale, pos.z + scale),
                Vec3(pos.x + scale, pos.y + scale, pos.z + scale)
            };

            switch (operation)
            {
                case BooleanOperation::UNION:

                    // At least one shape intersects the cube
                    for (const auto& shape : shapes)
                    {
                        if (shape->Intersects(pos, scale))
                        {
                            return true;
                        }
                    }

                    return false;

                case BooleanOperation::INTERSECTION:

                    // For intersection, we can quickly check if any shape completely misses
                    // the cube, in which case the result is empty
                    for (const auto& shape : shapes)
                    {
                        if (!shape->Intersects(pos, scale))
                        {
                            return false;
                        }
                    }

                    // If all shapes intersect the cube, we need a more detailed check
                    // because the intersections might not overlap

                    // If any corner is in all shapes, the intersection intersects the cube
                    for (const auto& corner : corners)
                    {
                        bool inAllShapes = true;

                        for (const auto& shape : shapes)
                        {
                            if (!shape->Contains(corner))
                            {
                                inAllShapes = false;
                                break;
                            }
                        }

                        if (inAllShapes)
                        {
                            return true;
                        }
                    }

                    // If no corner is in all shapes, we'll check if the first shape has
                    // a portion inside the cube that doesn't intersect other shapes
                    // This is a conservative approximation
                    return shapes[0]->Intersects(pos, scale);

                case BooleanOperation::DIFFERENCE:

                    // First shape must intersect the cube
                    if (!shapes[0]->Intersects(pos, scale))
                    {
                        return false;
                    }

                    // If any subtracted shape completely contains the cube, then no intersection
                    for (size_t i = 1; i < shapes.size(); i++)
                    {
                        if (shapes[i]->ContainsCompletely(pos, scale))
                        {
                            return false;
                        }
                    }

                    // Check if any corner of the cube is in the first shape but not in other shapes
                    for (const auto& corner : corners)
                    {
                        if (shapes[0]->Contains(corner))
                        {
                            bool inOtherShape = false;

                            for (size_t i = 1; i < shapes.size(); i++)
                            {
                                if (shapes[i]->Contains(corner))
                                {
                                    inOtherShape = true;
                                    break;
                                }
                            }

                            if (!inOtherShape)
                            {
                                return true;
                            }
                        }
                    }

                    // This is a conservative approximation
                    return true;

                default:
                    return false;
            }
        }

        bool ContainsCompletely(const Vec3& pos, int scale) const override
        {
            // Empty shape list
            if (shapes.empty())
            {
                return false;
            }

            // Single shape optimization
            if (shapes.size() == 1)
            {
                return shapes[0]->ContainsCompletely(pos, scale);
            }

            switch (operation)
            {
                case BooleanOperation::UNION:

                    // For a union to completely contain the cube, at least one shape must
                    // completely contain it
                    for (const auto& shape : shapes)
                    {
                        if (shape->ContainsCompletely(pos, scale))
                        {
                            return true;
                        }
                    }

                    return false;

                case BooleanOperation::INTERSECTION:

                    // For an intersection to completely contain the cube, all shapes must
                    // completely contain it
                    for (const auto& shape : shapes)
                    {
                        if (!shape->ContainsCompletely(pos, scale))
                        {
                            return false;
                        }
                    }

                    return true;

                case BooleanOperation::DIFFERENCE:

                    // For a difference to completely contain the cube:
                    // 1. The first shape must completely contain it
                    if (!shapes[0]->ContainsCompletely(pos, scale))
                    {
                        return false;
                    }

                    // 2. No other shape can intersect the cube
                    for (size_t i = 1; i < shapes.size(); i++)
                    {
                        if (shapes[i]->Intersects(pos, scale))
                        {
                            return false;
                        }
                    }

                    return true;

                default:
                    return false;
            }
        }
};