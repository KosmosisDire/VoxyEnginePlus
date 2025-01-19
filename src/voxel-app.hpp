#pragma once
#include "user-interface\root.hpp"
#include "vox/shaders/shared.inl"

#include "vox/vox-camera.hpp"
#include "vox/vox-renderer.hpp"
#include <application.hpp>
#include <input.hpp>
#include <stdio.h>

struct VoxyApp : public Application
{
  public:
    VoxelRenderer voxelRenderer;
    VoxCamera camera;
    UIData uiState;
    daxa::ImageId render_image;
    daxa::TaskImage task_render_image;

    VoxyApp()
        : Application("Voxy", {"../../../../src/vox/shaders"}), // use this path instead for dist: "resources/shaders"
          voxelRenderer(renderer),
          uiState({.renderData = &voxelRenderer.stateData})
    {
        render_image = renderer->CreateRenderImage("game_render_image", &task_render_image);
        camera.speed = 16.0f;
    }

    ~VoxyApp()
    {
        renderer->DestroyImage(render_image);

        ClayState::FreeAllStrings();
    }

  protected:
    void OnStart() override
    {
        voxelRenderer.InitializeTasks(renderer->render_loop_graph, &task_render_image, &render_image);
        renderer->Complete();
    }

    void OnUpdate(float dt) override
    {
        // renderer state
        RenderData &renderData = voxelRenderer.stateData;
        renderData.camera = camera.getCameraData();
        renderData.dt = dt;
        renderData.time = GetTime();
        renderData.frame++;

        // ui state
        uiState.time = GetTime();

        fflush(stdout);

        UIInputs input =
            {
                .screenWidth = (int)window->GetWidth(),
                .screenHeight = (int)window->GetHeight(),
                .pointerX = InputManager::GetMousePosition().x,
                .pointerY = InputManager::GetMousePosition().y,
                .pointerDown = InputManager::IsMouseButtonPressed(MouseButton::Left),
                .scrollDeltaX = InputManager::GetMouseScrollDelta().x,
                .scrollDeltaY = InputManager::GetMouseScrollDelta().y,
                .deltaTime = dt,
            };

        if (InputManager::WasKeyPressed(Key::F4))
        {
            ClayUI::DebugMode(!ClayUI::GetDebugMode());
        }

        if (InputManager::WasKeyPressed(Key::R))
        {
            voxelRenderer.dirtyTerrain = true;
        }

        // if pressing L, sun dir is locked to camera
        if (InputManager::IsKeyPressed(Key::L))
        {
            renderData.sunDir = to_daxa(-camera.getForward());
        }

        // mouse capture
        if (InputManager::WasMouseButtonPressed(MouseButton::Left) && !ImGui::GetIO().WantCaptureMouse && !uiState.mouseIsActive)
        {
            InputManager::CaptureMouseResetDelta(true);
        }

        if (InputManager::WasKeyPressed(Key::Escape))
        {
            InputManager::CaptureMouseResetDelta(!InputManager::IsMouseCaptured()); // use escape to toggle mouse capture
        }

        // if mouse is captured, update camera
        if (InputManager::IsMouseCaptured())
        {
            auto delta = InputManager::GetMouseDelta();
            camera.processMouseMovement(delta, true);
            camera.processKeyboard(dt);
        }

        uiState.mouseIsActive = !InputManager::IsMouseCaptured(); // assuming that the mouse is active when it is not captured (so it is visible and can interact with UI)
        bool menuOpenBefore = uiState.mouseIsActive;
        ClayUI::Update<UIData>(uiState, input, init_root_ui);

        if (menuOpenBefore && !uiState.mouseIsActive)
        {
            InputManager::CaptureMouseResetDelta(true);
        }

        voxelRenderer.Update();
    }

    void OnResize(u32 sx, u32 sy) override
    {
        voxelRenderer.Resize(sx, sy);
        renderer->DestroyImage(render_image);
        render_image = renderer->CreateRenderImage("game_render_image");
        task_render_image.set_images({.images = std::array{render_image}});
        camera.setViewportSize(sx, sy);

    }
};