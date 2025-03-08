#include <angelscript.h>
#include <engine/data/Vector3.hpp>

void Vector3Constructor(void* self)
{
    new (self) Vector3();
}

void Vector3ConstructorParam(float x, float y, float z, void* self)
{
    new (self) Vector3(x, y, z);
}

void registerVector3(asIScriptEngine *engine)
{
    int r = 0;
    r = engine->RegisterObjectType("Vector3", sizeof(Vector3), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Vector3>()); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vector3Constructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(float x, float y, float z)", asFUNCTION(Vector3ConstructorParam), asCALL_CDECL_OBJLAST); assert(r >= 0);

    r = engine->RegisterObjectProperty("Vector3", "float x", asOFFSET(Vector3, x)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Vector3", "float y", asOFFSET(Vector3, y)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Vector3", "float z", asOFFSET(Vector3, z)); assert(r >= 0);

    r = engine->RegisterObjectMethod("Vector3", "float magnitude()", asMETHOD(Vector3, magnitude), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "float sqrMagnitude()", asMETHOD(Vector3, sqrMagnitude), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "Vector3 normalized()", asMETHOD(Vector3, normalized), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "void normalize()", asMETHOD(Vector3, normalize), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "float dot(Vector3 other)", asMETHOD(Vector3, dot), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "Vector3 cross(Vector3 other)", asMETHOD(Vector3, cross), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "void scale(Vector3 by)", asMETHODPR(Vector3, scale, (const Vector3 &), void), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "string toString()", asMETHOD(Vector3, toString), asCALL_THISCALL); assert(r >= 0);

    r = engine->SetDefaultNamespace("Vector3"); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float angle(Vector3 from, Vector3 to)", asFUNCTION(Vector3::angle), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float signedAngle(Vector3 from, Vector3 to, Vector3 axis)", asFUNCTION(Vector3::signedAngle), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float distance(Vector3 a, Vector3 b)", asFUNCTION(Vector3::distance), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 lerp(Vector3 a, Vector3 b, float t)", asFUNCTION(Vector3::lerp), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 lerpUnclamped(Vector3 a, Vector3 b, float t)", asFUNCTION(Vector3::lerpUnclamped), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 moveTowards(Vector3 a, Vector3 b, float maxDelta)", asFUNCTION(Vector3::moveTowards), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 reflect(Vector3 vector, Vector3 normal)", asFUNCTION(Vector3::reflect), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 project(Vector3 vector, Vector3 onNormal)", asFUNCTION(Vector3::project), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 projectOnPlane(Vector3 vector, Vector3 planeNormal)", asFUNCTION(Vector3::projectOnPlane), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 scale(Vector3 a, Vector3 b)", asFUNCTIONPR(Vector3::scale, (const Vector3 &, const Vector3 &), Vector3), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 zero()", asFUNCTION(Vector3::zero), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 one()", asFUNCTION(Vector3::one), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 up()", asFUNCTION(Vector3::up), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 down()", asFUNCTION(Vector3::down), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 left()", asFUNCTION(Vector3::left), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 right()", asFUNCTION(Vector3::right), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 forward()", asFUNCTION(Vector3::forward), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector3 back()", asFUNCTION(Vector3::back), asCALL_STDCALL); assert(r >= 0);
    r = engine->SetDefaultNamespace(""); assert(r >= 0);

    r = // operators assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "Vector3 opAdd(Vector3 other)", asMETHODPR(Vector3, operator+, (const Vector3 &) const, Vector3), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "Vector3 opSub(Vector3 other)", asMETHODPR(Vector3, operator-, (const Vector3 &) const, Vector3), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "Vector3 opMul(float other)", asMETHODPR(Vector3, operator*, (float) const, Vector3), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "Vector3 opDiv(float other)", asMETHODPR(Vector3, operator/, (float) const, Vector3), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "Vector3 opAddAssign(Vector3 other)", asMETHODPR(Vector3, operator+=, (const Vector3 &), Vector3 &), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "Vector3 opSubAssign(Vector3 other)", asMETHODPR(Vector3, operator-=, (const Vector3 &), Vector3 &), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "Vector3 opMulAssign(float other)", asMETHODPR(Vector3, operator*=, (float), Vector3 &), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "Vector3 opDivAssign(float other)", asMETHODPR(Vector3, operator/=, (float), Vector3 &), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "Vector3 opNeg()", asMETHODPR(Vector3, operator-, () const, Vector3), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "bool opEquals(Vector3 other)", asMETHODPR(Vector3, operator==, (const Vector3 &) const, bool), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vector3", "bool opNotEquals(Vector3 other)", asMETHODPR(Vector3, operator!=, (const Vector3 &) const, bool), asCALL_THISCALL); assert(r >= 0);
}