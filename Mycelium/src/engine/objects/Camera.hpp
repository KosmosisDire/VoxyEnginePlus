#pragma once
#include <engine/apis/Input.hpp>
#include <engine/objects/Transform.hpp>
#include <engine/data/Matrix4x4.hpp>
#include <engine/apis/Math.hpp>
#include <engine/data/Vector3.hpp>
#include "shaders/engine.inl"

using namespace daxa::types;

class Camera
{
    public:
        float speed = 100.0f;
        float sensitivity = 0.1f;

        Camera(
            Vector3 position = Vector3(0.0f, 0.0f, 0.0f),
            Vector3 up = Vector3(0.0f, 1.0f, 0.0f),
            float yaw = -90.0f,
            float pitch = 0.0f,
            uint32_t initialWidth = 800,
            uint32_t initialHeight = 600,
            float fovDegrees = 45.0f,
            float nearPlane = 0.1f,
            float farPlane = 100.0f)
            : worldUp(up),
              yaw(yaw),
              pitch(pitch),
              viewportWidth(initialWidth),
              viewportHeight(initialHeight),
              fov(fovDegrees),
              near(nearPlane),
              far(farPlane),
              refCount(1)
        {
            validateAndUpdatePlanes(near, far);
            validateAndUpdateFOV(fov);
            transform.SetPosition(position);
            updateCameraVectors();
        }

        // Reference counting methods
        void AddRef()
        {
            refCount++;
        }

        void Release()
        {
            if(--refCount == 0)
                delete this;
        }

        // Viewport control
        void setViewportSize(uint32_t width, uint32_t height)
        {
            viewportWidth = width;
            viewportHeight = height;
        }

        float getAspectRatio() const
        {
            return static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
        }

        // FOV control
        void setFOV(float fovDegrees)
        {
            validateAndUpdateFOV(fovDegrees);
        }

        float getFOV() const { return fov; }

        // Near/Far plane control
        void setNearPlane(float nearPlane)
        {
            validateAndUpdatePlanes(nearPlane, far);
        }

        void setFarPlane(float farPlane)
        {
            validateAndUpdatePlanes(near, farPlane);
        }

        void setNearFarPlanes(float nearPlane, float farPlane)
        {
            validateAndUpdatePlanes(nearPlane, farPlane);
        }

        Matrix4x4 getProjectionMatrix() const
        {
            auto proj = Matrix4x4::perspective(fov, getAspectRatio(), near, far);
            proj(1, 1) *= -1; // Flip Y coordinate for Vulkan convention
            return proj;
        }

        Matrix4x4 getViewMatrix() const
        {
            return Matrix4x4::lookAt(getPosition(), getPosition() + getForward(), getUp());
        }

        float getNearPlane() const { return near; }

        float getFarPlane() const { return far; }

        Vector3 getForward() const { return transform.GetForward(); }

        Vector3 getRight() const { return transform.GetRight(); }

        Vector3 getUp() const { return transform.GetUp(); }

        Vector3 getPosition() const { return transform.GetPosition(); }

        // Transform access methods - return handles to the Transform
        Transform* getTransform()
        {
            transform.AddRef(); // Increment reference count before returning
            return &transform;
        }

        void processKeyboard(float deltaTime)
        {
            float velocity = speed * deltaTime;

            if (Input::IsKeyPressed(Key::W))
            {
                transform.Translate(getForward() * velocity);
            }

            if (Input::IsKeyPressed(Key::S))
            {
                transform.Translate(getForward() * -velocity);
            }

            if (Input::IsKeyPressed(Key::A))
            {
                transform.Translate(getRight() * -velocity);
            }

            if (Input::IsKeyPressed(Key::D))
            {
                transform.Translate(getRight() * velocity);
            }

            if (Input::IsKeyPressed(Key::Space))
            {
                transform.Translate(getUp() * velocity);
            }

            if (Input::IsKeyPressed(Key::LeftShift))
            {
                transform.Translate(getUp() * -velocity);
            }
        }

        void processMouseMovement(Vector2 mouseDelta, bool constrainPitch = true)
        {
            float xoffset = mouseDelta.x;
            float yoffset = mouseDelta.y;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            yaw += xoffset;
            pitch += yoffset;

            if (constrainPitch)
            {
                if (pitch > 89.0f)
                {
                    pitch = 89.0f;
                }

                if (pitch < -89.0f)
                {
                    pitch = -89.0f;
                }
            }

            updateCameraVectors();
        }

        CameraData getCameraData()
        {
            CameraData data;
            auto proj = getProjectionMatrix();
            auto view = getViewMatrix();

            // Combine view and projection
            auto viewProj = proj * view;

            data.viewProj = viewProj.toDaxa();
            data.view = view.toDaxa();
            data.proj = proj.toDaxa();
            data.invViewProj = viewProj.inverse().toDaxa();
            data.position = getPosition().toDaxa();
            data.near = getNearPlane();
            data.far = getFarPlane();
            data.fov = getFOV();
            return data;
        }

    private:
        Transform transform;
        Vector3 worldUp;

        float yaw;
        float pitch;

        uint32_t viewportWidth;
        uint32_t viewportHeight;

        float fov;  // Stored in degrees
        float near; // Near plane distance
        float far;  // Far plane distance
        int refCount; // Reference counter for memory management

        void validateAndUpdateFOV(float newFOV)
        {
            // Clamp FOV between reasonable limits (e.g., 1 to 179 degrees)
            fov = std::clamp(newFOV, 1.0f, 179.0f);
        }

        void validateAndUpdatePlanes(float newNear, float newFar)
        {
            // Ensure near plane is positive and not too close to zero
            // Ensure far plane is greater than near plane
            if (newNear <= 0.0001f)
            {
                newNear = 0.0001f;
            }

            if (newFar <= newNear)
            {
                newFar = newNear * 1000.0f;    // Set far plane to 1000x near plane if invalid
            }

            near = newNear;
            far = newFar;
        }

        void updateCameraVectors()
        {
            // Adjust rotation for Vulkan's coordinate system
            Quaternion rot = Quaternion::euler(Vector3(pitch, yaw, 0.0f));

            // In Vulkan coordinate system, we need to flip certain axes
            Vector3 forwardVec(0.0f, 0.0f, -1.0f); // -Z is forward

            // Set the rotation that corresponds to our yaw and pitch
            transform.SetRotation(rot);
        }
};