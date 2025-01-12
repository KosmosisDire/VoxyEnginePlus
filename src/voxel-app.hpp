#pragma once
#include "user-interface\root.hpp"
#include "vox/shaders/shared.inl"

#include "vox/vox-camera.hpp"
#include <application.hpp>
#include <input.hpp>
#include <stdio.h>

struct VoxyApp : public Application
{
  public:
    StateData stateData =
        {
            .backgroundColor = {1.0f, 1.0f, 1.0f},
            .sunDir = {1.0f, 0.2f, 0.0f},
            .camera = {},
        };

    VoxCamera camera;

    UIData uiState{
        .time = 0.0f,
        .backgroundColor = stateData.backgroundColor
    };

    VoxyApp()
        : Application("Voxy", {"resources/shaders"})
    {
        // Initialize compute pipeline
        compute_pipeline = renderer.AddComputePipeline<ComputePush>("voxel_raymarch", "compute.slang");
        render_image = renderer.CreateRenderImage("game_render_image", task_render_image);

        // Initialize occupancy buffer
        renderer.CreateBuffer("chunk_occupancy", TREE_SIZE_CUBE / BITS_PER_BYTE, chunk_occupancy_buffer, task_chunk_occupancy_buffer);
        renderer.CreateBuffer("brick_occupancy", (TREE_SIZE_CUBE * TREE_SIZE_CUBE) / BITS_PER_BYTE, brick_occupancy_buffer, task_brick_occupancy_buffer);
        renderer.CreateBuffer<StateData>("state buffer", state_buffer, task_state_buffer);

        auto chunk_ptr = renderer.MapBufferAs<Occupancy>(chunk_occupancy_buffer);
        auto brick_ptr = renderer.MapBufferAs<Occupancy>(brick_occupancy_buffer);

        // Initialize chunk occupancy buffer
        for (usize i = 0; i < TREE_SIZE_CUBE / BITS_PER_BYTE / sizeof(u64); i++)
        {
            chunk_ptr->occupancy[i] = (long long)rand() * rand();
        }

        // Initialize brick occupancy buffer
        for (usize i = 0; i < (TREE_SIZE_CUBE * TREE_SIZE_CUBE) / BITS_PER_BYTE / sizeof(u64); i++)
        {
            brick_ptr->occupancy[i] = (long long)rand() * rand();
        }
    }

    ~VoxyApp()
    {
        renderer.DestroyImage(render_image);
        renderer.DestroyBuffer(chunk_occupancy_buffer);
        renderer.DestroyBuffer(brick_occupancy_buffer);
        renderer.DestroyBuffer(state_buffer);
        ClayState::FreeAllStrings();
    }

  protected:
    void OnStart() override
    {
        InitializeTasks(renderer.render_loop_graph);
        renderer.Complete();
    }

    void OnUpdate(float dt) override
    {
        // rotate sun around y axis
        stateData.sunDir = daxa_transform_normal(stateData.sunDir, daxa_mat_from_axis_angle({0.0f, 1.0f, 0.0f}, 0.01f * dt));

        // if pressing L, sun dir is locked to camera
        if (InputManager::IsKeyPressed(Key::L))
        {
            stateData.sunDir = to_daxa(-camera.getForward());
        }

        // mouse capture
        if (InputManager::WasMouseButtonPressed(MouseButton::Left) && !ImGui::GetIO().WantCaptureMouse && !uiState.mouseIsActive)
        {
            InputManager::CaptureMouseResetDelta(true);
        }

        if (InputManager::WasKeyPressed(Key::Escape))
        {
            InputManager::CaptureMouseResetDelta(false);
            
        }

        // if mouse is captured, update camera
        if (InputManager::IsMouseCaptured())
        {
            auto delta = InputManager::GetMouseDelta();
            camera.processMouseMovement(delta, true);
            camera.processKeyboard(dt);
        }

        uiState.time = GetTime();
        uiState.backgroundColor = stateData.backgroundColor;

        UIInputs input =
            {
                .screenWidth = (int)window.GetWidth(),
                .screenHeight = (int)window.GetHeight(),
                .pointerX = InputManager::GetMousePosition().x,
                .pointerY = InputManager::GetMousePosition().y,
                .pointerDown = InputManager::IsMouseButtonPressed(MouseButton::Left),
                .scrollDeltaX = InputManager::GetMouseScrollDelta().x,
                .scrollDeltaY = InputManager::GetMouseScrollDelta().y,
                .deltaTime = dt,
            };

        bool menuOpenBefore = uiState.mouseIsActive;
        ClayUI::Update<UIData>(uiState, input, init_root_ui);

        if (menuOpenBefore && !uiState.mouseIsActive) 
        {
            // inputs will be captured for graphics
            InputManager::CaptureMouseResetDelta(true);
        }
    }

    void OnResize(u32 sx, u32 sy) override
    {
        printf("Resizing to %d, %d\n", sx, sy);
        renderer.DestroyImage(render_image);
        render_image = renderer.CreateRenderImage("game_render_image");
        task_render_image.set_images({.images = std::array{render_image}});
        camera.setViewportSize(sx, sy);
    }

    void InitializeTasks(daxa::TaskGraph &task_graph)
    {
        task_graph.use_persistent_image(task_render_image);
        task_graph.use_persistent_buffer(task_chunk_occupancy_buffer);
        task_graph.use_persistent_buffer(task_brick_occupancy_buffer);
        task_graph.use_persistent_buffer(task_state_buffer);

        renderer.AddTask(
            InlineTask("voxel_render")
                .add_attachment(daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_render_image))
                .add_attachment(daxa::inl_attachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_chunk_occupancy_buffer))
                .add_attachment(daxa::inl_attachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_brick_occupancy_buffer))
                .add_attachment(daxa::inl_attachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_state_buffer))
                .set_task([this](daxa::TaskInterface ti)
                          {
            stateData.camera = camera.getCameraData();
            renderer.CopyToBuffer<StateData>(ti, stateData, ti.get(task_state_buffer), 0);

            ti.recorder.set_pipeline(*compute_pipeline);
            ti.recorder.push_constant(ComputePush{
                .image = render_image.default_view(),
                .chunk_occupancy_ptr = renderer.GetDeviceAddress(ti, task_chunk_occupancy_buffer, 0),
                .brick_occupancy_ptr = renderer.GetDeviceAddress(ti, task_brick_occupancy_buffer, 0),
                .state_ptr = renderer.GetDeviceAddress(ti, task_state_buffer, 0),
                .frame_dim = {renderer.surface_width, renderer.surface_height},
                .time = GetTime(),
            });
            ti.recorder.dispatch({(renderer.surface_width + 7) / 8, (renderer.surface_height + 7) / 8}); })
        );

        renderer.AddTask(renderer.CreateSwapchainBlitTask(task_render_image));
    }

  private:
    std::shared_ptr<daxa::ComputePipeline> compute_pipeline;
    daxa::ImageId render_image;
    daxa::TaskImage task_render_image;

    daxa::BufferId chunk_occupancy_buffer;
    daxa::BufferId brick_occupancy_buffer;
    daxa::BufferId state_buffer;
    daxa::TaskBuffer task_chunk_occupancy_buffer;
    daxa::TaskBuffer task_brick_occupancy_buffer;
    daxa::TaskBuffer task_state_buffer;

    // helpers
};