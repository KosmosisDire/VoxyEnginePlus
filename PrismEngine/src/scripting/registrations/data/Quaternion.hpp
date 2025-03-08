#include <angelscript.h>
#include <engine/data/Quaternion.hpp>

void QuaternionConstructor(void* self)
{
    new (self) Quaternion();
}

void QuaternionConstructorParams(float x, float y, float z, float w, void* self)
{
    new (self) Quaternion(x, y, z, w);
}

void registerQuaternion(asIScriptEngine *engine)
{
    int r = 0;

    // Register the Quaternion type
    r = engine->RegisterObjectType("Quaternion", sizeof(Quaternion), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Quaternion>()); assert(r >= 0);

    // Register constructors
    r = engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(QuaternionConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f(float, float, float, float)", asFUNCTION(QuaternionConstructorParams), asCALL_CDECL_OBJLAST); assert(r >= 0);

    // Register properties
    r = engine->RegisterObjectProperty("Quaternion", "float x", asOFFSET(Quaternion, x)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Quaternion", "float y", asOFFSET(Quaternion, y)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Quaternion", "float z", asOFFSET(Quaternion, z)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Quaternion", "float w", asOFFSET(Quaternion, w)); assert(r >= 0);

    // Register utility methods
    r = engine->RegisterObjectMethod("Quaternion", "float magnitude()", asMETHOD(Quaternion, magnitude), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "float sqrMagnitude()", asMETHOD(Quaternion, sqrMagnitude), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "Quaternion normalized()", asMETHOD(Quaternion, normalized), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "void normalize()", asMETHOD(Quaternion, normalize), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "float dot(Quaternion)", asMETHOD(Quaternion, dot), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "float inverse()", asMETHOD(Quaternion, inverse), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "Vector3 eulerAngles()", asMETHOD(Quaternion, eulerAngles), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "Vector3 getForward()", asMETHOD(Quaternion, getForward), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "Vector3 getUp()", asMETHOD(Quaternion, getUp), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "Vector3 getRight()", asMETHOD(Quaternion, getRight), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "string toString()", asMETHOD(Quaternion, toString), asCALL_THISCALL); assert(r >= 0);

    // Register operators
    r = engine->RegisterObjectMethod("Quaternion", "Quaternion opAdd(Quaternion)", asMETHODPR(Quaternion, operator+, (const Quaternion &) const, Quaternion), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "Quaternion opSub(Quaternion)", asMETHODPR(Quaternion, operator-, (const Quaternion &) const, Quaternion), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "Quaternion opMul(float)", asMETHODPR(Quaternion, operator*, (float) const, Quaternion), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "Quaternion opMul(Quaternion)", asMETHODPR(Quaternion, operator*, (const Quaternion &) const, Quaternion), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "Vector3 opMul(Vector3)", asMETHODPR(Quaternion, operator*, (const Vector3 &) const, Vector3), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "Quaternion opDiv(float)", asMETHODPR(Quaternion, operator/, (float) const, Quaternion), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "Quaternion opNeg()", asMETHODPR(Quaternion, operator-, () const, Quaternion), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "bool opEquals(Quaternion)", asMETHODPR(Quaternion, operator==, (const Quaternion &) const, bool), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Quaternion", "bool opNotEquals(Quaternion)", asMETHODPR(Quaternion, operator!=, (const Quaternion &) const, bool), asCALL_THISCALL); assert(r >= 0);

    // Register static methods in the Quaternion namespace
    r = engine->SetDefaultNamespace("Quaternion"); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Quaternion identity()", asFUNCTION(Quaternion::identity), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Quaternion angleAxis(float, Vector3)", asFUNCTION(Quaternion::angleAxis), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Quaternion euler(float, float, float)", asFUNCTIONPR(Quaternion::euler, (float, float, float), Quaternion), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Quaternion euler(Vector3)", asFUNCTIONPR(Quaternion::euler, (const Vector3 &), Quaternion), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Quaternion fromToRotation(Vector3, Vector3)", asFUNCTION(Quaternion::fromToRotation), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Quaternion lookRotation(Vector3, Vector3 = Vector3(0,1,0))", asFUNCTION(Quaternion::lookRotation), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float angle(Quaternion, Quaternion)", asFUNCTION(Quaternion::angle), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Quaternion lerp(Quaternion, Quaternion, float)", asFUNCTION(Quaternion::lerp), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Quaternion slerp(Quaternion, Quaternion, float)", asFUNCTION(Quaternion::slerp), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Quaternion rotateTowards(Quaternion, Quaternion, float)", asFUNCTION(Quaternion::rotateTowards), asCALL_STDCALL); assert(r >= 0);
    r = engine->SetDefaultNamespace(""); assert(r >= 0);
}