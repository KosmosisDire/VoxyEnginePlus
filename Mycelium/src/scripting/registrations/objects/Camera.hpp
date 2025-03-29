#include <angelscript.h>
#include <engine/objects/Camera.hpp>

// Factory functions
Camera* CameraFactory()
{
    return new Camera();
}

Camera* CameraFactoryWithParams(Vector3 position, Vector3 up, float yaw, float pitch,
                                uint32_t width, uint32_t height, float fov,
                                float nearPlane, float farPlane)
{
    return new Camera(position, up, yaw, pitch, width, height, fov, nearPlane, farPlane);
}

// Reference counting methods
void CameraAddRef(Camera* camera)
{
    camera->AddRef();
}

void CameraRelease(Camera* camera)
{
    camera->Release();
}

void registerCamera(asIScriptEngine *engine)
{
    int r = 0;

    // Register the Camera type as a reference type
    r = engine->RegisterObjectType("Camera", 0, asOBJ_REF); assert(r >= 0);

    // Register factory functions
    r = engine->RegisterObjectBehaviour("Camera", asBEHAVE_FACTORY, "Camera@ f()", asFUNCTION(CameraFactory), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Camera", asBEHAVE_FACTORY,
                                        "Camera@ f(Vector3 position, Vector3 up, float yaw, float pitch, uint width, uint height, float fov, float nearPlane, float farPlane)",
                                        asFUNCTION(CameraFactoryWithParams), asCALL_CDECL); assert(r >= 0);

    // Register reference counting behaviors
    r = engine->RegisterObjectBehaviour("Camera", asBEHAVE_ADDREF, "void f()", asFUNCTION(CameraAddRef), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Camera", asBEHAVE_RELEASE, "void f()", asFUNCTION(CameraRelease), asCALL_CDECL_OBJFIRST); assert(r >= 0);

    // Register properties
    r = engine->RegisterObjectProperty("Camera", "float speed", asOFFSET(Camera, speed)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Camera", "float sensitivity", asOFFSET(Camera, sensitivity)); assert(r >= 0);

    // Register viewport methods
    r = engine->RegisterObjectMethod("Camera", "void setViewportSize(uint width, uint height)", asMETHOD(Camera, setViewportSize), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Camera", "float getAspectRatio()", asMETHOD(Camera, getAspectRatio), asCALL_THISCALL); assert(r >= 0);

    // Register FOV methods
    r = engine->RegisterObjectMethod("Camera", "void setFOV(float degrees)", asMETHOD(Camera, setFOV), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Camera", "float getFOV()", asMETHOD(Camera, getFOV), asCALL_THISCALL); assert(r >= 0);

    // Register near/far plane methods
    r = engine->RegisterObjectMethod("Camera", "void setNearPlane(float near)", asMETHOD(Camera, setNearPlane), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Camera", "void setFarPlane(float far)", asMETHOD(Camera, setFarPlane), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Camera", "void setNearFarPlanes(float near, float far)", asMETHOD(Camera, setNearFarPlanes), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Camera", "float getNearPlane()", asMETHOD(Camera, getNearPlane), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Camera", "float getFarPlane()", asMETHOD(Camera, getFarPlane), asCALL_THISCALL); assert(r >= 0);

    // Register matrix methods
    r = engine->RegisterObjectMethod("Camera", "Matrix4x4 getProjectionMatrix()", asMETHOD(Camera, getProjectionMatrix), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Camera", "Matrix4x4 getViewMatrix()", asMETHOD(Camera, getViewMatrix), asCALL_THISCALL); assert(r >= 0);

    // Register transform access - now returns a handle
    r = engine->RegisterObjectMethod("Camera", "Transform@ getTransform()", asMETHODPR(Camera, getTransform, (), Transform*), asCALL_THISCALL); assert(r >= 0);

    // Register directional getters (convenience methods using the transform)
    r = engine->RegisterObjectMethod("Camera", "Vector3 getForward()", asMETHOD(Camera, getForward), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Camera", "Vector3 getRight()", asMETHOD(Camera, getRight), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Camera", "Vector3 getUp()", asMETHOD(Camera, getUp), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Camera", "Vector3 getPosition()", asMETHOD(Camera, getPosition), asCALL_THISCALL); assert(r >= 0);

    // Register input processing methods
    r = engine->RegisterObjectMethod("Camera", "void processKeyboard(float dt)", asMETHOD(Camera, processKeyboard), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Camera", "void processMouseMovement(Vector2 mouseDelta, bool constrainPitch = true)", asMETHOD(Camera, processMouseMovement), asCALL_THISCALL); assert(r >= 0);
}