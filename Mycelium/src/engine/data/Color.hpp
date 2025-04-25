#pragma once

#include <cmath>
#include <string>
#include <algorithm>
#include <daxa/daxa.hpp>

struct Color
{
    float r;
    float g;
    float b;
    float a;

    // Constructors
    constexpr Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}

    constexpr Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) {}

    constexpr Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

    constexpr Color(const Color& other) = default;

    // Static factory methods for common colors
    static constexpr Color Black() { return Color(0.0f, 0.0f, 0.0f); }

    static constexpr Color White() { return Color(1.0f, 1.0f, 1.0f); }

    static constexpr Color Red() { return Color(1.0f, 0.0f, 0.0f); }

    static constexpr Color Green() { return Color(0.0f, 1.0f, 0.0f); }

    static constexpr Color Blue() { return Color(0.0f, 0.0f, 1.0f); }

    static constexpr Color Yellow() { return Color(1.0f, 1.0f, 0.0f); }

    static constexpr Color Cyan() { return Color(0.0f, 1.0f, 1.0f); }

    static constexpr Color Magenta() { return Color(1.0f, 0.0f, 1.0f); }

    static constexpr Color Gray() { return Color(0.5f, 0.5f, 0.5f); }

    static constexpr Color Transparent() { return Color(0.0f, 0.0f, 0.0f, 0.0f); }

    // Basic operators
    constexpr Color operator+(const Color& rhs) const
    {
        return Color(r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a);
    }

    constexpr Color operator-(const Color& rhs) const
    {
        return Color(r - rhs.r, g - rhs.g, b - rhs.b, a - rhs.a);
    }

    constexpr Color operator*(float scalar) const
    {
        return Color(r * scalar, g * scalar, b * scalar, a * scalar);
    }

    constexpr Color operator*(const Color& rhs) const
    {
        return Color(r * rhs.r, g * rhs.g, b * rhs.b, a * rhs.a);
    }

    constexpr Color operator/(float scalar) const
    {
        float inv = 1.0f / scalar;
        return Color(r * inv, g * inv, b * inv, a * inv);
    }

    constexpr Color &operator+=(const Color& rhs)
    {
        r += rhs.r;
        g += rhs.g;
        b += rhs.b;
        a += rhs.a;
        return *this;
    }

    constexpr Color &operator-=(const Color& rhs)
    {
        r -= rhs.r;
        g -= rhs.g;
        b -= rhs.b;
        a -= rhs.a;
        return *this;
    }

    constexpr Color &operator*=(float scalar)
    {
        r *= scalar;
        g *= scalar;
        b *= scalar;
        a *= scalar;
        return *this;
    }

    constexpr Color &operator*=(const Color& rhs)
    {
        r *= rhs.r;
        g *= rhs.g;
        b *= rhs.b;
        a *= rhs.a;
        return *this;
    }

    constexpr Color &operator/=(float scalar)
    {
        float inv = 1.0f / scalar;
        r *= inv;
        g *= inv;
        b *= inv;
        a *= inv;
        return *this;
    }

    // Comparison operators
    constexpr bool operator==(const Color& rhs) const
    {
        return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
    }

    constexpr bool operator!=(const Color& rhs) const
    {
        return !(*this == rhs);
    }

    // Utility methods
    [[nodiscard]] constexpr Color Clamped() const
    {
        return Color(
                   std::clamp(r, 0.0f, 1.0f),
                   std::clamp(g, 0.0f, 1.0f),
                   std::clamp(b, 0.0f, 1.0f),
                   std::clamp(a, 0.0f, 1.0f)
               );
    }

    void Clamp()
    {
        r = std::clamp(r, 0.0f, 1.0f);
        g = std::clamp(g, 0.0f, 1.0f);
        b = std::clamp(b, 0.0f, 1.0f);
        a = std::clamp(a, 0.0f, 1.0f);
    }

    // Color space conversions
    [[nodiscard]] static Color FromHSV(float h, float s, float v, float a = 1.0f)
    {
        h = std::fmod(h, 360.0f);

        if (h < 0.0f) h += 360.0f;

        s = std::clamp(s, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);

        float c = v * s;
        float x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
        float m = v - c;

        float r = 0.0f, g = 0.0f, b = 0.0f;

        if (h >= 0.0f && h < 60.0f)
        {
            r = c; g = x; b = 0.0f;
        }
        else if (h >= 60.0f && h < 120.0f)
        {
            r = x; g = c; b = 0.0f;
        }
        else if (h >= 120.0f && h < 180.0f)
        {
            r = 0.0f; g = c; b = x;
        }
        else if (h >= 180.0f && h < 240.0f)
        {
            r = 0.0f; g = x; b = c;
        }
        else if (h >= 240.0f && h < 300.0f)
        {
            r = x; g = 0.0f; b = c;
        }
        else
        {
            r = c; g = 0.0f; b = x;
        }

        return Color(r + m, g + m, b + m, a);
    }

    [[nodiscard]] void ToHSV(float& h, float& s, float& v) const
    {
        float cmax = std::max(std::max(r, g), b);
        float cmin = std::min(std::min(r, g), b);
        float delta = cmax - cmin;

        // Hue calculation
        if (delta < 1e-6f)
        {
            h = 0.0f;
        }
        else if (cmax == r)
        {
            h = 60.0f * std::fmod((g - b) / delta, 6.0f);
        }
        else if (cmax == g)
        {
            h = 60.0f * ((b - r) / delta + 2.0f);
        }
        else
        {
            h = 60.0f * ((r - g) / delta + 4.0f);
        }

        if (h < 0.0f) h += 360.0f;

        // Saturation calculation
        s = (cmax < 1e-6f) ? 0.0f : delta / cmax;

        // Value calculation
        v = cmax;
    }

    // Color blending
    [[nodiscard]] static Color Lerp(const Color& a, const Color& b, float t)
    {
        t = std::clamp(t, 0.0f, 1.0f);
        return Color(
                   a.r + (b.r - a.r) * t,
                   a.g + (b.g - a.g) * t,
                   a.b + (b.b - a.b) * t,
                   a.a + (b.a - a.a) * t
               );
    }

    [[nodiscard]] static Color LerpUnclamped(const Color& a, const Color& b, float t)
    {
        return Color(
                   a.r + (b.r - a.r) * t,
                   a.g + (b.g - a.g) * t,
                   a.b + (b.b - a.b) * t,
                   a.a + (b.a - a.a) * t
               );
    }

    // Alpha blending
    [[nodiscard]] Color alphaBlend(const Color& background) const
    {
        float outAlpha = a + background.a * (1.0f - a);

        if (outAlpha < 1e-6f)
        {
            return Color::Transparent();
        }

        float invOutAlpha = 1.0f / outAlpha;

        return Color(
                   (r * a + background.r * background.a * (1.0f - a)) * invOutAlpha,
                   (g * a + background.g * background.a * (1.0f - a)) * invOutAlpha,
                   (b * a + background.b * background.a * (1.0f - a)) * invOutAlpha,
                   outAlpha
               );
    }

    // Color utilities
    [[nodiscard]] Color grayscale() const
    {
        // Using luminance formula: 0.299R + 0.587G + 0.114B
        float gray = r * 0.299f + g * 0.587f + b * 0.114f;
        return Color(gray, gray, gray, a);
    }

    [[nodiscard]] Color inverted() const
    {
        return Color(1.0f - r, 1.0f - g, 1.0f - b, a);
    }

    [[nodiscard]] Color withAlpha(float newAlpha) const
    {
        return Color(r, g, b, newAlpha);
    }

    // Color adjustment methods
    [[nodiscard]] Color adjustBrightness(float factor) const
    {
        factor = std::max(0.0f, factor); // Ensure factor is not negative
        return Color(r * factor, g * factor, b * factor, a);
    }

    [[nodiscard]] Color adjustSaturation(float factor) const
    {
        float gray = r * 0.299f + g * 0.587f + b * 0.114f;

        return Color(
                   gray + (r - gray) * factor,
                   gray + (g - gray) * factor,
                   gray + (b - gray) * factor,
                   a
               );
    }

    [[nodiscard]] Color adjustHue(float degrees) const
    {
        float h, s, v;
        ToHSV(h, s, v);

        h += degrees;

        while (h >= 360.0f) h -= 360.0f;

        while (h < 0.0f) h += 360.0f;

        return FromHSV(h, s, v, a);
    }

    // Integer color conversions
    [[nodiscard]] static Color fromRGBA8(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
    {
        return Color(
                   static_cast<float>(r) / 255.0f,
                   static_cast<float>(g) / 255.0f,
                   static_cast<float>(b) / 255.0f,
                   static_cast<float>(a) / 255.0f
               );
    }

    [[nodiscard]] static Color fromHex(uint32_t hexValue)
    {
        return Color(
                   static_cast<float>((hexValue >> 16) & 0xFF) / 255.0f,
                   static_cast<float>((hexValue >> 8) & 0xFF) / 255.0f,
                   static_cast<float>(hexValue & 0xFF) / 255.0f,
                   static_cast<float>((hexValue >> 24) & 0xFF) / 255.0f
               );
    }

    [[nodiscard]] uint32_t toRGBA8() const
    {
        uint8_t r8 = static_cast<uint8_t>(std::clamp(r * 255.0f, 0.0f, 255.0f));
        uint8_t g8 = static_cast<uint8_t>(std::clamp(g * 255.0f, 0.0f, 255.0f));
        uint8_t b8 = static_cast<uint8_t>(std::clamp(b * 255.0f, 0.0f, 255.0f));
        uint8_t a8 = static_cast<uint8_t>(std::clamp(a * 255.0f, 0.0f, 255.0f));

        return (static_cast<uint32_t>(r8) << 24) |
               (static_cast<uint32_t>(g8) << 16) |
               (static_cast<uint32_t>(b8) << 8) |
               static_cast<uint32_t>(a8);
    }

    // String conversion
    [[nodiscard]] std::string toString() const
    {
        return "RGBA(" + std::to_string(r) + ", "
               + std::to_string(g) + ", " + std::to_string(b) + ", "
               + std::to_string(a) + ")";
    }

    [[nodiscard]] std::string toHexString() const
    {
        uint8_t r8 = static_cast<uint8_t>(std::clamp(r * 255.0f, 0.0f, 255.0f));
        uint8_t g8 = static_cast<uint8_t>(std::clamp(g * 255.0f, 0.0f, 255.0f));
        uint8_t b8 = static_cast<uint8_t>(std::clamp(b * 255.0f, 0.0f, 255.0f));
        uint8_t a8 = static_cast<uint8_t>(std::clamp(a * 255.0f, 0.0f, 255.0f));

        char buffer[10];
        std::snprintf(buffer, sizeof(buffer), "#%02X%02X%02X%02X", r8, g8, b8, a8);
        return std::string(buffer);
    }

    // Daxa conversions
    [[nodiscard]] daxa_f32vec4 toDaxa() const
    {
        return daxa_f32vec4{r, g, b, a};
    }

    static Color fromDaxa(const daxa_f32vec4& v)
    {
        return Color(v.x, v.y, v.z, v.w);
    }
};

// Global operators
inline constexpr Color operator*(float scalar, const Color& color)
{
    return color * scalar;
}