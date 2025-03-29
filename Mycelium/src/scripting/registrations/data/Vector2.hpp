#include <angelscript.h>
#include <engine/data/Vector2.hpp>

void Vector2Constructor(void* self)
{
    new (self) Vector2();
}

void Vector2ConstructorParam(float x, float y, void* self)
{
    new (self) Vector2(x, y);
}

void registerVector2(asIScriptEngine *engine)
{
    int r = 0;
    r = engine->RegisterObjectType("Vector2", sizeof(Vector2), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Vector2>()); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vector2Constructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f(float x, float y)", asFUNCTION(Vector2ConstructorParam), asCALL_CDECL_OBJLAST); assert(r >= 0);
    r = engine->RegisterObjectProperty("Vector2", "float x", asOFFSET(Vector2, x)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Vector2", "float y", asOFFSET(Vector2, y)); assert(r >= 0);

    r = engine->RegisterObjectMethod("Vector2", "float magnitude()", asMETHOD(Vector2, magnitude), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "float sqrMagnitude()", asMETHOD(Vector2, sqrMagnitude), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "Vector2 normalized()", asMETHOD(Vector2, normalized), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "void normalize()", asMETHOD(Vector2, normalize), asCALL_THISCALL); assert(r >=  0);
    r = engine->RegisterObjectMethod("Vector2", "float dot(Vector2 other)", asMETHOD(Vector2, dot), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "void scale(Vector2 by)", asMETHODPR(Vector2, scale, (const Vector2 &), void), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "string toString()", asMETHOD(Vector2, toString), asCALL_THISCALL); assert(r >= 0);

    r = engine->SetDefaultNamespace("Vector2"); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float angle(Vector2 a, Vector2 b)", asFUNCTION(Vector2::angle), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float distance(Vector2 a, Vector2 b)", asFUNCTION(Vector2::distance), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector2 lerp(Vector2 a, Vector2 b, float t)", asFUNCTION(Vector2::lerp), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector2 lerpUnclamped(Vector2 a, Vector2 b, float t)", asFUNCTION(Vector2::lerpUnclamped), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector2 moveTowards(Vector2 a, Vector2 b, float maxDelta)", asFUNCTION(Vector2::moveTowards), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float reflect(Vector2 vector, Vector2 normal)", asFUNCTION(Vector2::reflect), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector2 scale(Vector2 vector, Vector2 by)", asFUNCTIONPR(Vector2::scale, (const Vector2 &, const Vector2 &), Vector2), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector2 zero()", asFUNCTION(Vector2::zero), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector2 one()", asFUNCTION(Vector2::one), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector2 up()", asFUNCTION(Vector2::up), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector2 down()", asFUNCTION(Vector2::down), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector2 left()", asFUNCTION(Vector2::left), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector2 right()", asFUNCTION(Vector2::right), asCALL_STDCALL); assert(r >= 0);
    r = engine->SetDefaultNamespace(""); assert(r >= 0);

    r = engine->RegisterObjectMethod("Vector2", "Vector2 opAdd(Vector2 other)", asMETHODPR(Vector2, operator+, (const Vector2 &) const, Vector2), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "Vector2 opSub(Vector2 other)", asMETHODPR(Vector2, operator-, (const Vector2 &) const, Vector2), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "Vector2 opMul(float other)", asMETHODPR(Vector2, operator*, (float) const, Vector2), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "Vector2 opDiv(float other)", asMETHODPR(Vector2, operator/, (float) const, Vector2), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "Vector2 opAddAssign(Vector2 other)", asMETHODPR(Vector2, operator+=, (const Vector2 &), Vector2 &), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "Vector2 opSubAssign(Vector2 other)", asMETHODPR(Vector2, operator-=, (const Vector2 &), Vector2 &), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "Vector2 opMulAssign(float other)", asMETHODPR(Vector2, operator*=, (float), Vector2 &), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "Vector2 opDivAssign(float other)", asMETHODPR(Vector2, operator/=, (float), Vector2 &), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "Vector2 opNeg()", asMETHODPR(Vector2, operator-, () const, Vector2), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "bool opEquals(Vector2 other)", asMETHODPR(Vector2, operator==, (const Vector2 &) const, bool), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector2", "bool opNotEquals(Vector2 other)", asMETHODPR(Vector2, operator!=, (const Vector2 &) const, bool), asCALL_THISCALL); assert(r >= 0);

}