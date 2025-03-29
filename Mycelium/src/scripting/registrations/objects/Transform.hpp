#include <angelscript.h>
#include <engine/objects/Transform.hpp>

// Factory functions
Transform* TransformFactory()
{
    return new Transform();
}

Transform* TransformFactoryWithPosition(Vector3 position)
{
    return new Transform(position);
}

Transform* TransformFactoryWithPositionRotation(Vector3 position, Quaternion rotation)
{
    return new Transform(position, rotation);
}

Transform* TransformFactoryWithPositionRotationScale(Vector3 position, Quaternion rotation, Vector3 scale)
{
    return new Transform(position, rotation, scale);
}

// Reference counting methods
void TransformAddRef(Transform* transform)
{
    transform->AddRef();
}

void TransformRelease(Transform* transform)
{
    transform->Release();
}

void registerTransform(asIScriptEngine *engine)
{
    int r = 0;

    // Register the Transform type as a reference type
    r = engine->RegisterObjectType("Transform", 0, asOBJ_REF); assert(r >= 0);

    // Register the Space enum
    r = engine->SetDefaultNamespace("Transform"); assert(r >= 0);
    r = engine->RegisterEnum("Space"); assert(r >= 0);
    r = engine->RegisterEnumValue("Space", "Self", static_cast<int>(Transform::Space::Self)); assert(r >= 0);
    r = engine->RegisterEnumValue("Space", "World", static_cast<int>(Transform::Space::World)); assert(r >= 0);
    r = engine->SetDefaultNamespace(""); assert(r >= 0);

    // Register factory functions
    r = engine->RegisterObjectBehaviour("Transform", asBEHAVE_FACTORY, "Transform@ f()", asFUNCTION(TransformFactory), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Transform", asBEHAVE_FACTORY, "Transform@ f(Vector3 position)", asFUNCTION(TransformFactoryWithPosition), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Transform", asBEHAVE_FACTORY, "Transform@ f(Vector3 position, Quaternion rotation)", asFUNCTION(TransformFactoryWithPositionRotation), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Transform", asBEHAVE_FACTORY, "Transform@ f(Vector3 position, Quaternion rotation, Vector3 scale)", asFUNCTION(TransformFactoryWithPositionRotationScale), asCALL_CDECL); assert(r >= 0);

    // Register reference counting behaviors
    r = engine->RegisterObjectBehaviour("Transform", asBEHAVE_ADDREF, "void f()", asFUNCTION(TransformAddRef), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Transform", asBEHAVE_RELEASE, "void f()", asFUNCTION(TransformRelease), asCALL_CDECL_OBJFIRST); assert(r >= 0);

    // Register Position methods
    r = engine->RegisterObjectMethod("Transform", "Vector3 GetPosition() const", asMETHOD(Transform, GetPosition), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "void SetPosition(const Vector3 &in position)", asMETHOD(Transform, SetPosition), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "void Translate(const Vector3 &in translation)", asMETHODPR(Transform, Translate, (const Vector3&), void), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "void Translate(const Vector3 &in translation, Transform::Space relativeTo)", asMETHODPR(Transform, Translate, (const Vector3&, Transform::Space), void), asCALL_THISCALL); assert(r >= 0);

    // Register Rotation methods
    r = engine->RegisterObjectMethod("Transform", "Quaternion GetRotation() const", asMETHOD(Transform, GetRotation), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "void SetRotation(const Quaternion &in rotation)", asMETHOD(Transform, SetRotation), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "void Rotate(const Vector3 &in eulerAngles)", asMETHODPR(Transform, Rotate, (const Vector3&), void), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "void Rotate(const Vector3 &in axis, float angle)", asMETHODPR(Transform, Rotate, (const Vector3&, float), void), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "void Rotate(const Quaternion &in rotation)", asMETHODPR(Transform, Rotate, (const Quaternion&), void), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "void LookAt(const Vector3 &in target, const Vector3 &in worldUp = Vector3(0,1,0))", asMETHOD(Transform, LookAt), asCALL_THISCALL); assert(r >= 0);

    // Register Scale methods
    r = engine->RegisterObjectMethod("Transform", "Vector3 GetScale() const", asMETHOD(Transform, GetScale), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "void SetScale(const Vector3 &in scale)", asMETHODPR(Transform, SetScale, (const Vector3&), void), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "void SetScale(float uniformScale)", asMETHODPR(Transform, SetScale, (float), void), asCALL_THISCALL); assert(r >= 0);

    // Register local axis getters
    r = engine->RegisterObjectMethod("Transform", "Vector3 GetForward() const", asMETHOD(Transform, GetForward), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "Vector3 GetRight() const", asMETHOD(Transform, GetRight), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "Vector3 GetUp() const", asMETHOD(Transform, GetUp), asCALL_THISCALL); assert(r >= 0);

    // Register matrix operations
    r = engine->RegisterObjectMethod("Transform", "Matrix4x4 GetLocalToWorldMatrix() const", asMETHOD(Transform, GetLocalToWorldMatrix), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "Matrix4x4 GetWorldToLocalMatrix() const", asMETHOD(Transform, GetWorldToLocalMatrix), asCALL_THISCALL); assert(r >= 0);

    // Register hierarchy operations
    r = engine->RegisterObjectMethod("Transform", "Transform@ GetParent() const", asMETHOD(Transform, GetParent), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "void SetParent(Transform@ newParent)", asMETHOD(Transform, SetParent), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "Vector3 TransformPoint(const Vector3 &in point) const", asMETHOD(Transform, TransformPoint), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "Vector3 TransformDirection(const Vector3 &in direction) const", asMETHOD(Transform, TransformDirection), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "Vector3 InverseTransformPoint(const Vector3 &in worldPoint) const", asMETHOD(Transform, InverseTransformPoint), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Transform", "Vector3 InverseTransformDirection(const Vector3 &in worldDirection) const", asMETHOD(Transform, InverseTransformDirection), asCALL_THISCALL); assert(r >= 0);
}