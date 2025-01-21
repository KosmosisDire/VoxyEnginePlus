#pragma once

#include "shaders/shared.inl"
#include <camera.hpp>

class VoxCamera : public Camera
{
  public:
    CameraData getCameraData()
    {
        CameraData data;
        auto proj = getProjectionMatrix();
        auto view = getViewMatrix();

        // Combine view and projection
        auto viewProj = proj * view;

        data.viewProj = to_daxa(viewProj);
        data.invViewProj = to_daxa(inverse(viewProj));
        data.position = to_daxa(getPosition());
        data.near = getNearPlane();
        data.far = getFarPlane();
        data.fov = getFOV();
        return data;
    }
};