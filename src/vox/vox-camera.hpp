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

        data.viewProj = viewProj.toDaxa();
        data.invViewProj = viewProj.inverse().toDaxa();
        data.position = getPosition().toDaxa();
        data.near = getNearPlane();
        data.far = getFarPlane();
        data.fov = getFOV();
        return data;
    }
};