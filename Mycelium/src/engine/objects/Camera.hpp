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
            validate_and_update_planes(near, far); // Rename call
            validate_and_update_fov(fov); // Rename call
            transform.set_position(position); // Update call from Transform.hpp changes
            update_camera_vectors(); // Rename call
        }

        // Reference counting methods
        void add_ref()
        {
            refCount++;
        }

        void release()
        {
            if(--refCount == 0)
                delete this;
        }

        // Viewport control
        void set_viewport_size(uint32_t width, uint32_t height)
        {
            viewportWidth = width;
            viewportHeight = height;
        }

        float get_aspect_ratio() const
        {
            return static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
        }

        // FOV control
        void set_fov(float fovDegrees)
        {
            validate_and_update_fov(fovDegrees); // Rename call
        }

        float get_fov() const { return fov; }

        // Near/Far plane control
        void set_near_plane(float nearPlane)
        {
            validate_and_update_planes(nearPlane, far); // Rename call
        }

        void set_far_plane(float farPlane)
        {
            validate_and_update_planes(near, farPlane); // Rename call
        }

        void set_near_far_planes(float nearPlane, float farPlane)
        {
            validate_and_update_planes(nearPlane, farPlane); // Rename call
        }

        Matrix4x4 get_projection_matrix() const
        {
            auto proj = Matrix4x4::perspective(fov, get_aspect_ratio(), near, far); // Update call
            proj(1, 1) *= -1; // Flip Y coordinate for Vulkan convention
            return proj;
        }

        Matrix4x4 get_view_matrix() const
        {
            return Matrix4x4::look_at(get_position(), get_position() + get_forward(), get_up()); // Update calls
        }

        float get_near_plane() const { return near; }

        float get_far_plane() const { return far; }

        Vector3 get_forward() const { return transform.get_forward(); } // Update call

        Vector3 get_right() const { return transform.get_right(); } // Update call

        Vector3 get_up() const { return transform.get_up(); } // Update call

        Vector3 get_position() const { return transform.get_position(); } // Update call

        // Transform access methods - return handles to the Transform
        Transform* get_transform()
        {
            transform.add_ref(); // Update call
            return &transform;
        }

        void process_keyboard(float deltaTime)
        {
            float velocity = speed * deltaTime;

            if (Input::is_key_pressed(Key::W))
            {
                transform.translate(get_forward() * velocity); // Update call
            }

            if (Input::is_key_pressed(Key::S))
            {
                transform.translate(get_forward() * -velocity); // Update call
            }

            if (Input::is_key_pressed(Key::A))
            {
                transform.translate(get_right() * -velocity); // Update call
            }

            if (Input::is_key_pressed(Key::D))
            {
                transform.translate(get_right() * velocity); // Update call
            }

            if (Input::is_key_pressed(Key::Space))
            {
                transform.translate(get_up() * velocity); // Update call
            }

            if (Input::is_key_pressed(Key::LeftShift))
            {
                transform.translate(get_up() * -velocity); // Update call
            }
        }

        void process_mouse_movement(Vector2 mouseDelta, bool constrainPitch = true)
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

            update_camera_vectors(); // Rename call
        }

        CameraData get_camera_data()
        {
            CameraData data;
            auto proj = get_projection_matrix(); // Update call
            auto view = get_view_matrix(); // Update call

            // Combine view and projection
            auto viewProj = proj * view;

            data.viewProj = viewProj.to_daxa();
            data.view = view.to_daxa();
            data.proj = proj.to_daxa();
            data.invViewProj = viewProj.inverse().to_daxa();
            data.position = get_position().to_daxa(); // Update call
            data.near = get_near_plane(); // Update call
            data.far = get_far_plane(); // Update call
            data.fov = get_fov(); // Update call
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

        void validate_and_update_fov(float newFOV)
        {
            // Clamp FOV between reasonable limits (e.g., 1 to 179 degrees)
            fov = std::clamp(newFOV, 1.0f, 179.0f);
        }

        void validate_and_update_planes(float newNear, float newFar)
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

        void update_camera_vectors()
        {
            // Adjust rotation for Vulkan's coordinate system
            Quaternion rot = Quaternion::euler(Vector3(pitch, yaw, 0.0f));

            // In Vulkan coordinate system, we need to flip certain axes
            Vector3 forwardVec(0.0f, 0.0f, -1.0f); // -Z is forward

            // Set the rotation that corresponds to our yaw and pitch
            transform.set_rotation(rot); // Update call
        }
};
