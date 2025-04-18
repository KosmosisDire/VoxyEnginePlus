#pragma once

#include "user-interface/root.hpp"
#include "vox/shaders/shared.inl"
#include <scripting/scripting.hpp>

#include "vox/vox-camera.hpp"
#include "vox/vox-renderer.hpp"
#include <engine/objects/Application.hpp>
#include <engine/apis/Input.hpp>
#include <project_paths.h>
#include <stdio.h>

struct VoxyApp : public Application
{
    public:
        VoxelRenderer voxelRenderer;
        UIData uiState;
        daxa::ImageId render_image;
        daxa::TaskImage task_render_image;

        VoxyApp()
            : Application("Voxy", {"resources/shaders"}), // use this path instead for dist: "resources/shaders"

        voxelRenderer(renderer),
        uiState({.renderData = &voxelRenderer.stateData})
        {
            render_image = renderer->CreateRenderImage("game_render_image", &task_render_image);
            voxelRenderer.camera.speed = 6.0f;
            voxelRenderer.camera.sensitivity = 0.05f;

            scriptingEngine.registerProperty("Vector3 sunDir", &voxelRenderer.stateData.sunDir);
            scriptingEngine.registerProperty("Camera camera", &voxelRenderer.camera);
            scriptingEngine.registerProperty("float dt", &voxelRenderer.stateData.dt);
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

            scripts->executeFunction("void Update()");
            voxelRenderer.Update();
        }

        void OnResize(u32 sx, u32 sy) override
        {
            voxelRenderer.Resize(sx, sy);
            renderer->DestroyImage(render_image);
            render_image = renderer->CreateRenderImage("game_render_image");
            task_render_image.set_images({.images = std::array{render_image}});
        }
};