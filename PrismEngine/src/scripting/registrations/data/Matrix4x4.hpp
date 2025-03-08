#include <angelscript.h>
#include <engine/data/Matrix4x4.hpp>

void Matrix4x4Constructor(void* self)
{
    new (self) Matrix4x4();
}

void Matrix4x4Destructor(void* self)
{
    ((Matrix4x4*)self)->~Matrix4x4();
}

// Array access helper for script access
float Matrix4x4Get(int row, int col, Matrix4x4 *self)
{
    return (*self)(row, col);
}

void Matrix4x4Set(int row, int col, float value, Matrix4x4 *self)
{
    (*self)(row, col) = value;
}

void registerMatrix4x4(asIScriptEngine *engine)
{
    int r = 0;

    // Register the Matrix4x4 as Matrix4x4 in AngelScript
    r = engine->RegisterObjectType("Matrix4x4", sizeof(Matrix4x4), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert(r >= 0);

    // Register constructors
    r = engine->RegisterObjectBehaviour("Matrix4x4", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Matrix4x4Constructor), asCALL_CDECL_OBJLAST); assert(r >= 0);

    // Register destructor
    r = engine->RegisterObjectBehaviour("Matrix4x4", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Matrix4x4Destructor), asCALL_CDECL_OBJLAST); assert(r >= 0);

    // Element access
    r = engine->RegisterObjectMethod("Matrix4x4", "float get(int x, int y)", asFUNCTION(Matrix4x4Get), asCALL_CDECL_OBJLAST); assert(r >= 0);
    r = engine->RegisterObjectMethod("Matrix4x4", "void set(int x, int y, float value)", asFUNCTION(Matrix4x4Set), asCALL_CDECL_OBJLAST); assert(r >= 0);

    // Basic operators
    r = engine->RegisterObjectMethod("Matrix4x4", "Matrix4x4 opMul(Matrix4x4 other)", asMETHODPR(Matrix4x4, operator*, (const Matrix4x4 &) const, Matrix4x4), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Matrix4x4", "Vector4 opMul(Vector4 other)", asMETHODPR(Matrix4x4, operator*, (const Vector4 &) const, Vector4), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Matrix4x4", "bool opEquals(Matrix4x4 other)", asMETHODPR(Matrix4x4, operator==, (const Matrix4x4 &) const, bool), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Matrix4x4", "bool opNotEquals(Matrix4x4 other)", asMETHODPR(Matrix4x4, operator!=, (const Matrix4x4 &) const, bool), asCALL_THISCALL); assert(r >= 0);

    // Vector transformation methods
    r = engine->RegisterObjectMethod("Matrix4x4", "Vector3 transformPoint(Vector3 point)", asMETHOD(Matrix4x4, transformPoint), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Matrix4x4", "Vector3 transformPointAffine(Vector3 point)", asMETHOD(Matrix4x4, transformPointAffine), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Matrix4x4", "Vector3 transformDirection(Vector3 direction)", asMETHOD(Matrix4x4, transformDirection), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Matrix4x4", "Vector3 transformNormal(Vector3 normal)", asMETHOD(Matrix4x4, transformNormal), asCALL_THISCALL); assert(r >= 0);

    // Matrix operations
    r = engine->RegisterObjectMethod("Matrix4x4", "Matrix4x4 inverse()", asMETHOD(Matrix4x4, inverse), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Matrix4x4", "Matrix4x4 transpose()", asMETHOD(Matrix4x4, transpose), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Matrix4x4", "float determinant()", asMETHOD(Matrix4x4, determinant), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Matrix4x4", "void decompose(Vector3&out translation, Quaternion&out rotation, Vector3&out scale)", asMETHOD(Matrix4x4, decompose), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Matrix4x4", "string toString()", asMETHOD(Matrix4x4, toString), asCALL_THISCALL); assert(r >= 0);

    // Static factory methods
    r = engine->SetDefaultNamespace("Matrix4x4"); assert(r >= 0);

    r = engine->RegisterGlobalFunction("Matrix4x4 identity()", asFUNCTION(Matrix4x4::identity), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Matrix4x4 translation(Vector3 translation)", asFUNCTION(Matrix4x4::translation), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Matrix4x4 rotation(Quaternion rotation)", asFUNCTION(Matrix4x4::rotation), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Matrix4x4 scale(Vector3 scale)", asFUNCTION(Matrix4x4::scale), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Matrix4x4 TRS(Vector3 translation, Quaternion rotation, Vector3 scale)", asFUNCTION(Matrix4x4::TRS), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Matrix4x4 perspective(float fovY, float aspect, float near, float far)", asFUNCTION(Matrix4x4::perspective), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Matrix4x4 orthographic(float left, float right, float bottom, float top, float near, float far)", asFUNCTION(Matrix4x4::orthographic), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Matrix4x4 lookAt(Vector3 position, Vector3 target, Vector3 up)", asFUNCTION(Matrix4x4::lookAt), asCALL_STDCALL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Matrix4x4 fromAxisAngle(Vector3 axis, float angle)", asFUNCTION(Matrix4x4::fromAxisAngle), asCALL_STDCALL); assert(r >= 0);

    r = engine->SetDefaultNamespace(""); assert(r >= 0);
}