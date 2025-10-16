#pragma once

#include "user-interface/root.hpp"
#include <shaders/shared.inl>

#include "voxels/vox-renderer.hpp"
#include <engine/objects/Application.hpp>
#include <engine/apis/Input.hpp>
#include <stdio.h>

struct VoxyApp : public Application
{
    public:
        VoxelRenderer voxelRenderer;
        UIData uiState;
        daxa::ImageId render_image;
        daxa::TaskImage task_render_image;

        VoxyApp()
            : Application("Voxy", {"../../../../PrismEngine/resources/shaders", "../../../../Mycelium/resources/shaders"}),
        voxelRenderer(renderer),
        uiState({.renderData = &voxelRenderer.stateData})
        {
            render_image = renderer->CreateRenderImage("game_render_image", &task_render_image);
            voxelRenderer.camera.speed = 6.0f;
            voxelRenderer.camera.sensitivity = 0.05f;
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
                .pointerX = Input::GetMousePosition().x,
                .pointerY = Input::GetMousePosition().y,
                .pointerDown = Input::IsMouseButtonPressed(MouseButton::Left),
                .scrollDeltaX = Input::GetMouseScrollDelta().x,
                .scrollDeltaY = Input::GetMouseScrollDelta().y,
                .deltaTime = dt,
            };

            if (Input::WasKeyPressed(Key::F4))
            {
                ClayUI::DebugMode(!ClayUI::GetDebugMode());
            }

            uiState.mouseIsActive = !Input::IsMouseCaptured(); // assuming that the mouse is active when it is not captured (so it is visible and can interact with UI)
            bool menuOpenBefore = uiState.mouseIsActive;
            ClayUI::Update<UIData>(uiState, input, draw_root_ui);

            if (menuOpenBefore && !uiState.mouseIsActive)
            {
                Input::CaptureMouseResetDelta(true);
            }

            // Camera and input logic (previously from test.as script)
            HandleCameraControls(dt);

            voxelRenderer.Update();
        }

    private:
        void HandleCameraControls(float dt)
        {
            // Set sun direction to camera forward when L is pressed
            if (Input::IsKeyPressed(Key::L))
            {
                voxelRenderer.stateData.sunDir = (-voxelRenderer.camera.getForward()).toDaxa();
            }

            // Capture mouse when clicking while not captured
            if (!Input::IsMouseCaptured() && Input::IsMouseButtonPressed(MouseButton::Left))
            {
                Input::CaptureMouse(true);
            }

            // Release mouse when pressing Escape while captured
            if (Input::IsMouseCaptured() && Input::IsKeyPressed(Key::Escape))
            {
                Input::CaptureMouse(false);
            }

            // Process camera movement when mouse is captured
            if (Input::IsMouseCaptured())
            {
                voxelRenderer.camera.speed = 30;
                voxelRenderer.camera.processMouseMovement(-Input::GetMouseDelta(), true);
                voxelRenderer.camera.processKeyboard(dt);
            }
        }

        void OnResize(u32 sx, u32 sy) override
        {
            voxelRenderer.Resize(sx, sy);
            renderer->DestroyImage(render_image);
            render_image = renderer->CreateRenderImage("game_render_image");
            task_render_image.set_images({.images = std::array{render_image}});
        }
};