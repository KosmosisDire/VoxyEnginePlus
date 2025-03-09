#pragma once
#include "user-interface/root.hpp"
#include "vox/shaders/shared.inl"
#include <scripting/scripting.hpp>

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

        VoxyApp(): Application("Voxy", {"resources/shaders"}),

                voxelRenderer(renderer),
                uiState()
        {
            render_image = renderer->CreateRenderImage("game_render_image", &task_render_image);
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
            uiState.time = GetTime();
            voxelRenderer.time = GetTime();


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

            scripts->executeFunction("void Update(float dt)", dt);
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