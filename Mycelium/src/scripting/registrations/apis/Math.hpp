#include <angelscript.h>
#include <engine/apis/Math.hpp>

void registerBasicMath(asIScriptEngine *engine)
{
    int r = 0;
    r = engine->RegisterGlobalProperty("const float PI", const_cast<float*>(&Math::PI)); assert(r >= 0);

    r = engine->RegisterGlobalFunction("float abs(float x)", asFUNCTIONPR(Math::abs, (float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float sqrt(float x)", asFUNCTIONPR(Math::sqrt, (float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float pow(float x, float p)", asFUNCTIONPR(Math::pow, (float, float), float), asCALL_CDECL); assert(r >= 0);

    r = engine->RegisterGlobalFunction("float ceil(float x)", asFUNCTIONPR(Math::ceil, (float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float floor(float x)", asFUNCTIONPR(Math::floor, (float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float round(float x)", asFUNCTIONPR(Math::round, (float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float clamp(float x, float minVal, float maxVal)", asFUNCTIONPR(Math::clamp, (float, float, float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float clamp01(float x)", asFUNCTIONPR(Math::clamp01, (float), float), asCALL_CDECL); assert(r >= 0);

    r = engine->RegisterGlobalFunction("float lerp(float a, float b, float t)", asFUNCTIONPR(Math::lerp, (float, float, float), float), asCALL_CDECL); assert(r >= 0);

    // Min/Max functions
    // Register only for float and int types as templates can't be directly registered
    r = engine->RegisterGlobalFunction("float min(float a, float b)", asFUNCTIONPR(Math::min<float>, (float, float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float max(float a, float b)", asFUNCTIONPR(Math::max<float>, (float, float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("int min(int a, int b)", asFUNCTIONPR(Math::min<int>, (int, int), int), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("int max(int a, int b)", asFUNCTIONPR(Math::max<int>, (int, int), int), asCALL_CDECL); assert(r >= 0);
}

void registerMath(asIScriptEngine *engine)
{
    int r = 0;

    // register basic math outside of the Math namespace as well
    registerBasicMath(engine);

    // Register constants in the Math namespace
    r = engine->SetDefaultNamespace("Math"); assert(r >= 0);

    registerBasicMath(engine);
    r = engine->RegisterGlobalProperty("const float Deg2Rad", const_cast<float*>(&Math::Deg2Rad)); assert(r >= 0);
    r = engine->RegisterGlobalProperty("const float Rad2Deg", const_cast<float*>(&Math::Rad2Deg)); assert(r >= 0);
    r = engine->RegisterGlobalProperty("const float Epsilon", const_cast<float*>(&Math::Epsilon)); assert(r >= 0);
    r = engine->RegisterGlobalProperty("const float Infinity", const_cast<float*>(&Math::Infinity)); assert(r >= 0);
    r = engine->RegisterGlobalProperty("const float NegativeInfinity", const_cast<float*>(&Math::NegativeInfinity)); assert(r >= 0);

    // Angular conversion functions
    r = engine->RegisterGlobalFunction("float radians(float deg)", asFUNCTIONPR(Math::radians, (float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float degrees(float rad)", asFUNCTIONPR(Math::degrees, (float), float), asCALL_CDECL); assert(r >= 0);

    // Trigonometric functions
    r = engine->RegisterGlobalFunction("float sin(float x)", asFUNCTIONPR(Math::sin, (float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float cos(float x)", asFUNCTIONPR(Math::cos, (float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float tan(float x)", asFUNCTIONPR(Math::tan, (float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float asin(float x)", asFUNCTIONPR(Math::asin, (float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float acos(float x)", asFUNCTIONPR(Math::acos, (float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float atan(float x)", asFUNCTIONPR(Math::atan, (float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float atan2(float y, float x)", asFUNCTIONPR(Math::atan2, (float, float), float), asCALL_CDECL); assert(r >= 0);

    // Rounding functions
    r = engine->RegisterGlobalFunction("int ceilToInt(float x)", asFUNCTIONPR(Math::ceilToInt, (float), int), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("int floorToInt(float x)", asFUNCTIONPR(Math::floorToInt, (float), int), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("int roundToInt(float x)", asFUNCTIONPR(Math::roundToInt, (float), int), asCALL_CDECL); assert(r >= 0);

    // Value repeating functions
    r = engine->RegisterGlobalFunction("float repeat(float t, float length)", asFUNCTIONPR(Math::repeat, (float, float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float pingPong(float t, float length)", asFUNCTIONPR(Math::pingPong, (float, float), float), asCALL_CDECL); assert(r >= 0);

    // Interpolation functions
    r = engine->RegisterGlobalFunction("float lerpUnclamped(float value1, float value2, float t)", asFUNCTIONPR(Math::lerpUnclamped, (float, float, float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float lerpAngle(float a, float b, float t)", asFUNCTIONPR(Math::lerpAngle, (float, float, float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float inverseLerp(float a, float b, float t)", asFUNCTIONPR(Math::inverseLerp, (float, float, float), float), asCALL_CDECL); assert(r >= 0);

    // Angle functions
    r = engine->RegisterGlobalFunction("float deltaAngle(float current, float target)", asFUNCTIONPR(Math::deltaAngle, (float, float), float), asCALL_CDECL); assert(r >= 0);

    // Movement functions
    r = engine->RegisterGlobalFunction("float moveTowards(float current, float target, float maxDelta)", asFUNCTIONPR(Math::moveTowards, (float, float, float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float moveTowardsAngle(float current, float target, float maxDelta)", asFUNCTIONPR(Math::moveTowardsAngle, (float, float, float), float), asCALL_CDECL); assert(r >= 0);

    // Smoothing functions
    r = engine->RegisterGlobalFunction("float smoothStep(float from, float to, float t)", asFUNCTIONPR(Math::smoothStep, (float, float, float), float), asCALL_CDECL); assert(r >= 0);

    // Special handling for smoothDamp functions with reference parameters
    // We need to use a wrapper that handles the out parameter properly
    r = engine->RegisterGlobalFunction("float smoothDamp(float current, float target, float&out currentVelocity, float smoothTime, float maxSpeed = Math::Infinity, float deltaTime = 0.02f)",
                                       asFUNCTIONPR(Math::smoothDamp, (float, float, float&, float, float, float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float smoothDampAngle(float current, float target, float&out currentVelocity, float smoothTime, float maxSpeed = Math::Infinity, float deltaTime = 0.02f)",
                                       asFUNCTIONPR(Math::smoothDampAngle, (float, float, float&, float, float, float), float), asCALL_CDECL); assert(r >= 0);

    // Comparison functions
    r = engine->RegisterGlobalFunction("bool approximately(float a, float b)", asFUNCTIONPR(Math::approximately, (float, float), bool), asCALL_CDECL); assert(r >= 0);

    // Power of two functions
    r = engine->RegisterGlobalFunction("bool isPowerOfTwo(int x)", asFUNCTIONPR(Math::isPowerOfTwo, (int), bool), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("int nextPowerOfTwo(int x)", asFUNCTIONPR(Math::nextPowerOfTwo, (int), int), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("int closestPowerOfTwo(int x)", asFUNCTIONPR(Math::closestPowerOfTwo, (int), int), asCALL_CDECL); assert(r >= 0);

    // Color space conversion
    r = engine->RegisterGlobalFunction("float gammaToLinearSpace(float x)", asFUNCTIONPR(Math::gammaToLinearSpace, (float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float linearToGammaSpace(float x)", asFUNCTIONPR(Math::linearToGammaSpace, (float), float), asCALL_CDECL); assert(r >= 0);

    // Logarithm functions
    r = engine->RegisterGlobalFunction("float log(float x, float base)", asFUNCTIONPR(Math::log, (float, float), float), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float log10(float x)", asFUNCTIONPR(Math::log10, (float), float), asCALL_CDECL); assert(r >= 0);

    // Sign function
    r = engine->RegisterGlobalFunction("float sign(float x)", asFUNCTIONPR(Math::sign, (float), float), asCALL_CDECL); assert(r >= 0);

    // Reset namespace
    r = engine->SetDefaultNamespace(""); assert(r >= 0);
}