#include <angelscript.h>
#include <engine/data/Vector4.hpp>

void Vector4Constructor(void* self)
{
    new (self) Vector4();
}

void Vector4ConstructorParam(float x, float y, float z, float w, void* self)
{
    new (self) Vector4(x, y, z, w);
}

void registerVector4(asIScriptEngine *engine)
{
    int r = 0;

    // Register the Vector4 type as Vector4 in AngelScript
    r = engine->RegisterObjectType("Vector4", sizeof(Vector4), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Vector4>()); assert(r >= 0);

    // Register constructors
    r = engine->RegisterObjectBehaviour("Vector4", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vector4Constructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Vector4", asBEHAVE_CONSTRUCT, "void f(float x, float y, float z, float w)", asFUNCTION(Vector4ConstructorParam), asCALL_CDECL_OBJLAST); assert(r >= 0);

    // Register properties
    r = engine->RegisterObjectProperty("Vector4", "float x", asOFFSET(Vector4, x)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Vector4", "float y", asOFFSET(Vector4, y)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Vector4", "float z", asOFFSET(Vector4, z)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Vector4", "float w", asOFFSET(Vector4, w)); assert(r >= 0);

    // Register utility methods
    r = engine->RegisterObjectMethod("Vector4", "float magnitude()", asMETHOD(Vector4, magnitude), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "float sqrMagnitude()", asMETHOD(Vector4, sqrMagnitude), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "Vector4 normalized()", asMETHOD(Vector4, normalized), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "void normalize()", asMETHOD(Vector4, normalize), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "float dot(Vector4 other)", asMETHOD(Vector4, dot), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "void scale(Vector4 by)", asMETHODPR(Vector4, scale, (const Vector4 &), void), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "string toString()", asMETHOD(Vector4, toString), asCALL_THISCALL); assert(r >= 0);

    // Register static methods in Vector4 namespace
    r = engine->SetDefaultNamespace("Vector4"); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float distance(Vector4, Vector4)", asFUNCTION(Vector4::distance), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector4 lerp(Vector4, Vector4, float)", asFUNCTION(Vector4::lerp), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector4 lerpUnclamped(Vector4, Vector4, float)", asFUNCTION(Vector4::lerpUnclamped), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector4 moveTowards(Vector4, Vector4, float)", asFUNCTION(Vector4::moveTowards), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector4 scale(Vector4, Vector4)", asFUNCTIONPR(Vector4::scale, (const Vector4 &, const Vector4 &), Vector4), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector4 scale(float, Vector4)", asFUNCTIONPR(Vector4::scale, (const Vector4 &, const Vector4 &), Vector4), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector4 project(Vector4, Vector4)", asFUNCTION(Vector4::project), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector4 zero()", asFUNCTION(Vector4::zero), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector4 one()", asFUNCTION(Vector4::one), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector4 identity()", asFUNCTION(Vector4::identity), asCALL_STDCALL); assert(r >= 0);
    r = engine->SetDefaultNamespace(""); assert(r >= 0);

    // Register operators
    r = engine->RegisterObjectMethod("Vector4", "Vector4 opAdd(Vector4)", asMETHODPR(Vector4, operator+, (const Vector4 &) const, Vector4), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "Vector4 opSub(Vector4)", asMETHODPR(Vector4, operator-, (const Vector4 &) const, Vector4), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "Vector4 opMul(float)", asMETHODPR(Vector4, operator*, (float) const, Vector4), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "Vector4 opDiv(float)", asMETHODPR(Vector4, operator/, (float) const, Vector4), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "Vector4 opAddAssign(Vector4)", asMETHODPR(Vector4, operator+=, (const Vector4 &), Vector4 &), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "Vector4 opSubAssign(Vector4)", asMETHODPR(Vector4, operator-=, (const Vector4 &), Vector4 &), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "Vector4 opMulAssign(float)", asMETHODPR(Vector4, operator*=, (float), Vector4 &), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "Vector4 opDivAssign(float)", asMETHODPR(Vector4, operator/=, (float), Vector4 &), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "Vector4 opNeg()", asMETHODPR(Vector4, operator-, () const, Vector4), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "bool opEquals(Vector4)", asMETHODPR(Vector4, operator==, (const Vector4 &) const, bool), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector4", "bool opNotEquals(Vector4)", asMETHODPR(Vector4, operator!=, (const Vector4 &) const, bool), asCALL_THISCALL); assert(r >= 0);
}