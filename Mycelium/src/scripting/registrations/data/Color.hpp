#include <angelscript.h>
#include <engine/data/Color.hpp>

void ColorDefaultConstructor(void* self)
{
    new (self) Color();
}

void ColorConstructorRGB(float r, float g, float b, void* self)
{
    new (self) Color(r, g, b);
}

void ColorConstructorRGBA(float r, float g, float b, float a, void* self)
{
    new (self) Color(r, g, b, a);
}

// Helper for HSV to RGB conversion in scripts
Color ColorFromHSV(float h, float s, float v, float a)
{
    return Color::fromHSV(h, s, v, a);
}

// Helpers for integer-based color creation in scripts
Color ColorFromRGBA8(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return Color::fromRGBA8(r, g, b, a);
}

Color ColorFromHex(uint32_t hexValue)
{
    return Color::fromHex(hexValue);
}

// Helper for HSV conversion (output parameters)
void ColorToHSV(const Color &color, float& h, float& s, float& v)
{
    color.toHSV(h, s, v);
}

void registerColor(asIScriptEngine *engine)
{
    int r = 0;

    // Register the Color type
    r = engine->RegisterObjectType("Color", sizeof(Color), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Color>()); assert(r >= 0);

    // Register constructors
    r = engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ColorDefaultConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f(float r, float g, float b)", asFUNCTION(ColorConstructorRGB), asCALL_CDECL_OBJLAST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f(float r, float g, float b, float a)", asFUNCTION(ColorConstructorRGBA), asCALL_CDECL_OBJLAST); assert(r >= 0);

    // Register properties
    r = engine->RegisterObjectProperty("Color", "float r", asOFFSET(Color, r)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Color", "float g", asOFFSET(Color, g)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Color", "float b", asOFFSET(Color, b)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Color", "float a", asOFFSET(Color, a)); assert(r >= 0);

    // Register utility methods
    r = engine->RegisterObjectMethod("Color", "Color clamped() const", asMETHOD(Color, clamped), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "void clamp()", asMETHOD(Color, clamp), asCALL_THISCALL); assert(r >= 0);

    r = engine->RegisterObjectMethod("Color", "Color alphaBlend(const Color &in background) const", asMETHOD(Color, alphaBlend), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "Color grayscale() const", asMETHOD(Color, grayscale), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "Color inverted() const", asMETHOD(Color, inverted), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "Color withAlpha(float newAlpha) const", asMETHOD(Color, withAlpha), asCALL_THISCALL); assert(r >= 0);

    r = engine->RegisterObjectMethod("Color", "Color adjustBrightness(float factor) const", asMETHOD(Color, adjustBrightness), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "Color adjustSaturation(float factor) const", asMETHOD(Color, adjustSaturation), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "Color adjustHue(float degrees) const", asMETHOD(Color, adjustHue), asCALL_THISCALL); assert(r >= 0);

    r = engine->RegisterObjectMethod("Color", "uint toRGBA8() const", asMETHOD(Color, toRGBA8), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "string toString() const", asMETHOD(Color, toString), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "string toHexString() const", asMETHOD(Color, toHexString), asCALL_THISCALL); assert(r >= 0);

    // Register operators
    r = engine->RegisterObjectMethod("Color", "Color opAdd(const Color &in rhs) const", asMETHODPR(Color, operator+, (const Color&) const, Color), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "Color opSub(const Color &in rhs) const", asMETHODPR(Color, operator-, (const Color&) const, Color), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "Color opMul(float scalar) const", asMETHODPR(Color, operator*, (float) const, Color), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "Color opMul(const Color &in rhs) const", asMETHODPR(Color, operator*, (const Color&) const, Color), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "Color opDiv(float scalar) const", asMETHODPR(Color, operator/, (float) const, Color), asCALL_THISCALL); assert(r >= 0);

    r = engine->RegisterObjectMethod("Color", "Color &opAddAssign(const Color &in rhs)", asMETHODPR(Color, operator+=, (const Color&), Color&), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "Color &opSubAssign(const Color &in rhs)", asMETHODPR(Color, operator-=, (const Color&), Color&), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "Color &opMulAssign(float scalar)", asMETHODPR(Color, operator*=, (float), Color&), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "Color &opMulAssign(const Color &in rhs)", asMETHODPR(Color, operator*=, (const Color&), Color&), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "Color &opDivAssign(float scalar)", asMETHODPR(Color, operator/=, (float), Color&), asCALL_THISCALL); assert(r >= 0);

    r = engine->RegisterObjectMethod("Color", "bool opEquals(const Color &in rhs) const", asMETHODPR(Color, operator==, (const Color&) const, bool), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Color", "bool opNotEquals(const Color &in rhs) const", asMETHODPR(Color, operator!=, (const Color&) const, bool), asCALL_THISCALL); assert(r >= 0);

    // Register static methods in the Color namespace
    r = engine->SetDefaultNamespace("Color"); assert(r >= 0);

    // Predefined colors
    r = engine->RegisterGlobalFunction("Color black()", asFUNCTION(Color::black), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Color white()", asFUNCTION(Color::white), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Color red()", asFUNCTION(Color::red), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Color green()", asFUNCTION(Color::green), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Color blue()", asFUNCTION(Color::blue), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Color yellow()", asFUNCTION(Color::yellow), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Color cyan()", asFUNCTION(Color::cyan), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Color magenta()", asFUNCTION(Color::magenta), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Color gray()", asFUNCTION(Color::gray), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Color transparent()", asFUNCTION(Color::transparent), asCALL_STDCALL); assert(r >= 0);

    // Color utilities
    r = engine->RegisterGlobalFunction("Color fromHSV(float h, float s, float v, float a = 1.0f)", asFUNCTION(ColorFromHSV), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("void toHSV(const Color &in color, float &out h, float &out s, float &out v)", asFUNCTION(ColorToHSV), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Color fromRGBA8(uint8 r, uint8 g, uint8 b, uint8 a = 255)", asFUNCTION(ColorFromRGBA8), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Color fromHex(uint hexValue)", asFUNCTION(ColorFromHex), asCALL_CDECL); assert(r >= 0);

    // Blending functions
    r = engine->RegisterGlobalFunction("Color lerp(const Color &in a, const Color &in b, float t)", asFUNCTION(Color::lerp), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Color lerpUnclamped(const Color &in a, const Color &in b, float t)", asFUNCTION(Color::lerpUnclamped), asCALL_STDCALL); assert(r >= 0);

    r = engine->SetDefaultNamespace(""); assert(r >= 0);
}