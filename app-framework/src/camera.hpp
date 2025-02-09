#pragma once
#include "input.hpp"
#include "math/vector3.hpp"
#include "math/mat4x4.hpp"
#include "math/math.hpp"

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
        : position(position),
          worldUp(up),
          yaw(yaw),
          pitch(pitch),
          viewportWidth(initialWidth),
          viewportHeight(initialHeight),
          fov(fovDegrees),
          near(nearPlane),
          far(farPlane)
    {
        validateAndUpdatePlanes(near, far);
        validateAndUpdateFOV(fov);
        updateCameraVectors();
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
        proj(1,1) *= -1; // Flip Y coordinate for Vulkan convention
        return proj;
    }

    Matrix4x4 getViewMatrix() const
    {
        return Matrix4x4::lookAt(position, position + front, up);
    }

    float getNearPlane() const { return near; }
    float getFarPlane() const { return far; }
    Vector3 getForward() const { return front; }
    Vector3 getRight() const { return right; }
    Vector3 getUp() const { return up; }
    Vector3 getPosition() const { return position; }

    void processKeyboard(float deltaTime)
    {
        float velocity = speed * deltaTime;

        if (Input::IsKeyPressed(Key::W))
            position += front * velocity;
        if (Input::IsKeyPressed(Key::S))
            position -= front * velocity;
        if (Input::IsKeyPressed(Key::A))
            position -= right * velocity;
        if (Input::IsKeyPressed(Key::D))
            position += right * velocity;
        if (Input::IsKeyPressed(Key::Space))
            position += up * velocity;
        if (Input::IsKeyPressed(Key::LeftShift))
            position -= up * velocity;
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
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }

        updateCameraVectors();
    }

  private:
    Vector3 position;
    Vector3 front;
    Vector3 up;
    Vector3 right;
    Vector3 worldUp;

    float yaw;
    float pitch;

    uint32_t viewportWidth;
    uint32_t viewportHeight;

    float fov;  // Stored in degrees
    float near; // Near plane distance
    float far;  // Far plane distance

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
            newNear = 0.0001f;

        if (newFar <= newNear)
            newFar = newNear * 1000.0f; // Set far plane to 1000x near plane if invalid

        near = newNear;
        far = newFar;
    }

    void updateCameraVectors()
    {
        Vector3 newFront;
        // Adjust the calculations for Vulkan's coordinate system
        newFront.x = -Math::cos(Math::radians(yaw)) * Math::cos(Math::radians(pitch));
        newFront.y = -Math::sin(Math::radians(pitch)); // Flip Y since Vulkan Y points down
        newFront.z = -Math::sin(Math::radians(yaw)) * Math::cos(Math::radians(pitch));
        front = newFront.normalized();
        // Keep right-handed coordinate system
        right = front.cross(worldUp).normalized();
        up = right.cross(front).normalized();
    }
};