#pragma once
#include <cmath>
#include <algorithm>
#include <limits>

namespace Math {
    // Constants
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float Deg2Rad = 0.0174532925199432957692369076848861f;
    static constexpr float Rad2Deg = 57.295779513082320876798154814105f;
    static constexpr float Epsilon = std::numeric_limits<float>::epsilon();
    static constexpr float Infinity = std::numeric_limits<float>::infinity();
    static constexpr float NegativeInfinity = -std::numeric_limits<float>::infinity();

    // Already provided functions
    inline float radians(float degrees) {
        return degrees * Deg2Rad;
    }

    inline float degrees(float radians) {
        return radians * Rad2Deg;
    }

    // Basic math operations
    inline float abs(float f) {
        return std::abs(f);
    }

    inline float sqrt(float f) {
        return std::sqrt(f);
    }

    inline float pow(float f, float p) {
        return std::pow(f, p);
    }

    // Trigonometric functions
    inline float sin(float f) {
        return std::sin(f);
    }

    inline float cos(float f) {
        return std::cos(f);
    }

    inline float tan(float f) {
        return std::tan(f);
    }

    inline float asin(float f) {
        return std::asin(std::clamp(f, -1.0f, 1.0f));
    }

    inline float acos(float f) {
        return std::acos(std::clamp(f, -1.0f, 1.0f));
    }

    inline float atan(float f) {
        return std::atan(f);
    }

    inline float atan2(float y, float x) {
        return std::atan2(y, x);
    }

    // Rounding functions
    inline float ceil(float f) {
        return std::ceil(f);
    }

    inline int ceilToInt(float f) {
        return static_cast<int>(std::ceil(f));
    }

    inline float floor(float f) {
        return std::floor(f);
    }

    inline int floorToInt(float f) {
        return static_cast<int>(std::floor(f));
    }

    inline float round(float f) {
        return std::round(f);
    }

    inline int roundToInt(float f) {
        return static_cast<int>(std::round(f));
    }

    // Clamping functions
    inline float clamp(float value, float min, float max) {
        return std::clamp(value, min, max);
    }

    inline float clamp01(float value) {
        return std::clamp(value, 0.0f, 1.0f);
    }

    // Value repeating functions
    inline float repeat(float t, float length) {
        return std::clamp(t - std::floor(t / length) * length, 0.0f, length);
    }

    inline float pingPong(float t, float length) {
        t = repeat(t, length * 2.0f);
        return length - std::abs(t - length);
    }

    // Interpolation functions
    inline float lerp(float a, float b, float t) {
        return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
    }

    inline float lerpUnclamped(float a, float b, float t) {
        return a + (b - a) * t;
    }

    inline float lerpAngle(float a, float b, float t) {
        float delta = repeat((b - a), 360.0f);
        if (delta > 180.0f)
            delta -= 360.0f;
        return a + delta * std::clamp(t, 0.0f, 1.0f);
    }

    inline float inverseLerp(float a, float b, float value) {
        if (a != b)
            return std::clamp((value - a) / (b - a), 0.0f, 1.0f);
        return 0.0f;
    }

    // Angle functions
    inline float deltaAngle(float current, float target) {
        float delta = repeat((target - current), 360.0f);
        if (delta > 180.0f)
            delta -= 360.0f;
        return delta;
    }


    // Movement functions
    inline float moveTowards(float current, float target, float maxDelta) {
        if (std::abs(target - current) <= maxDelta)
            return target;
        return current + std::copysign(maxDelta, target - current);
    }

    inline float moveTowardsAngle(float current, float target, float maxDelta) {
        float delta = deltaAngle(current, target);
        if (-maxDelta < delta && delta < maxDelta)
            return target;
        target = current + delta;
        return moveTowards(current, target, maxDelta);
    }

    // Smoothing functions
    inline float smoothStep(float from, float to, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        t = t * t * (3.0f - 2.0f * t);
        return to * t + from * (1.0f - t);
    }

    inline float smoothDamp(float current, float target, float& currentVelocity, 
                          float smoothTime, float maxSpeed = Infinity, float deltaTime = 0.02f) {
        smoothTime = std::max(0.0001f, smoothTime);
        float omega = 2.0f / smoothTime;

        float x = omega * deltaTime;
        float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
        float change = current - target;
        float originalTo = target;

        float maxChange = maxSpeed * smoothTime;
        change = std::clamp(change, -maxChange, maxChange);
        target = current - change;

        float temp = (currentVelocity + omega * change) * deltaTime;
        currentVelocity = (currentVelocity - omega * temp) * exp;
        float output = target + (change + temp) * exp;

        if (originalTo - current > 0.0f == output > originalTo) {
            output = originalTo;
            currentVelocity = (output - originalTo) / deltaTime;
        }

        return output;
    }

    inline float smoothDampAngle(float current, float target, float& currentVelocity,
                               float smoothTime, float maxSpeed = Infinity, float deltaTime = 0.02f) {
        target = current + deltaAngle(current, target);
        return smoothDamp(current, target, currentVelocity, smoothTime, maxSpeed, deltaTime);
    }

    // Comparison functions
    inline bool approximately(float a, float b) {
        return std::abs(b - a) < std::max(0.000001f * std::max(std::abs(a), std::abs(b)), Epsilon * 8.0f);
    }

    // Power of two functions
    inline bool isPowerOfTwo(int value) {
        return value > 0 && (value & (value - 1)) == 0;
    }

    inline int nextPowerOfTwo(int value) {
        value--;
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        value++;
        return value;
    }

    inline int closestPowerOfTwo(int value) {
        int next = nextPowerOfTwo(value);
        int prev = next >> 1;
        return (value - prev) < (next - value) ? prev : next;
    }

    // Color space conversion
    inline float gammaToLinearSpace(float value) {
        if (value <= 0.04045f)
            return value / 12.92f;
        return std::pow((value + 0.055f) / 1.055f, 2.4f);
    }

    inline float linearToGammaSpace(float value) {
        if (value <= 0.0031308f)
            return value * 12.92f;
        return 1.055f * std::pow(value, 1.0f / 2.4f) - 0.055f;
    }

    // Min/Max functions
    template<typename T>
    inline T min(T a, T b) {
        return std::min(a, b);
    }

    template<typename T>
    inline T max(T a, T b) {
        return std::max(a, b);
    }

    // Logarithm functions
    inline float log(float f, float p) {
        return std::log(f) / std::log(p);
    }

    inline float log10(float f) {
        return std::log10(f);
    }

    // Sign function
    inline float sign(float f) {
        return f >= 0.0f ? 1.0f : -1.0f;
    }
}