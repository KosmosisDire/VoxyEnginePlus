#pragma once
#include "core/ui-build.hpp"
#include "vox/shaders/shared.inl"

#include <application.hpp>
#include <camera.hpp>
#include <functional>
#include <input.hpp>
#include <stdio.h>

using namespace daxa::types;

struct VoxyApp : public Application
{
  public:
    StateData stateData =
        {
            .backgroundColor = {1.0f, 1.0f, 1.0f},
            .sunDir = {1.0f, 0.2f, 0.0f},
            .camera = {},
        };

    Camera camera;

    UIState uiState{
        .time = 0.0f,
        .backgroundColor = stateData.backgroundColor,
        .menuOpen = true,
    };

    VoxyApp()
    {
        // Initialize compute pipeline
        compute_pipeline = pipeline_manager.add_compute_pipeline(
                                               {
                                                   .shader_info = {.source = daxa::ShaderFile{"compute.slang"}},
                                                   .push_constant_size = sizeof(ComputePush),
                                                   .name = "compute_pipeline",
                                               })
                               .value();

        // Create render image
        render_image = device.create_image({
            .format = daxa::Format::R8G8B8A8_UNORM,
            .size = {window->get_width(), window->get_height(), 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .name = "render_image",
        });

        // Initialize task render image
        task_render_image = daxa::TaskImage({.initial_images = {
                                                 .images = std::array{render_image}},
                                             .name = "task_render_image"});

        // Initialize occupancy buffer
        create_buffer(device, TREE_SIZE_CUBE / BITS_PER_BYTE, chunk_occupancy_buffer, task_chunk_occupancy_buffer, "chunk occupancy buffer");
        create_buffer(device, (TREE_SIZE_CUBE * TREE_SIZE_CUBE) / BITS_PER_BYTE, brick_occupancy_buffer, task_brick_occupancy_buffer, "brick occupancy buffer");
        create_buffer<StateData>(device, state_buffer, task_state_buffer, "state buffer");

        auto chunk_ptr = device.get_host_address_as<Occupancy>(chunk_occupancy_buffer).value();
        auto brick_ptr = device.get_host_address_as<Occupancy>(brick_occupancy_buffer).value();

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

        Application::init();
    }

    ~VoxyApp()
    {
        device.wait_idle();
        device.collect_garbage();
        device.destroy_image(render_image);
        device.destroy_buffer(chunk_occupancy_buffer);
        device.destroy_buffer(brick_occupancy_buffer);
        device.destroy_buffer(state_buffer);
        ClayState::FreeAllStrings();
    }

  protected:
    CameraData getCameraData(Camera cam)
    {
        CameraData data;
        auto proj = cam.getProjectionMatrix();
        auto view = cam.getViewMatrix();

        // Combine view and projection
        auto viewProj = proj * view;

        data.viewProj = to_daxa(viewProj);
        data.invViewProj = to_daxa(glm::inverse(viewProj));
        data.position = to_daxa(cam.getPosition());
        data.near = cam.getNearPlane();
        data.far = cam.getFarPlane();
        return data;
    }

    void on_update() override
    {
        // rotate sun around y axis
        stateData.sunDir = daxa_transform_normal(stateData.sunDir, daxa_mat_from_axis_angle({0.0f, 1.0f, 0.0f}, 0.01f * delta_time));

        // if pressing L, sun dir is locked to camera
        if (InputManager::IsKeyPressed(Key::L))
        {
            stateData.sunDir = to_daxa(-camera.getForward());
        }

        // mouse capture
        if (InputManager::WasMouseButtonPressed(MouseButton::Left) && !ImGui::GetIO().WantCaptureMouse && !uiState.menuOpen)
        {
            capture_mouse(true);
        }

        if (InputManager::WasKeyPressed(Key::Escape))
        {
            capture_mouse(false);
            uiState.menuOpen = !uiState.menuOpen;
        }

        // if mouse is captured, update camera
        if (is_mouse_captured())
        {
            float dx, dy;
            InputManager::GetMouseDelta(dx, dy);
            camera.processMouseMovement(dx, dy, true);
            camera.processKeyboard(delta_time);
        }

        uiState.time = time;
        uiState.backgroundColor = stateData.backgroundColor;

        UIInputs input =
            {
                .screenWidth = (int)window->get_width(),
                .screenHeight = (int)window->get_height(),
                .pointerX = InputManager::GetMousePositionX(),
                .pointerY = InputManager::GetMousePositionY(),
                .pointerDown = InputManager::IsMouseButtonPressed(MouseButton::Left),
                .scrollDeltaX = InputManager::GetMouseScrollDeltaX(),
                .scrollDeltaY = InputManager::GetMouseScrollDeltaY(),
                .deltaTime = delta_time,
            };

        bool menuOpenBefore = uiState.menuOpen;
        ClayState::Update<UIState>(uiState, input, build_ui);

        if (menuOpenBefore && !uiState.menuOpen)
        {
            capture_mouse(true);
        }
    }

    void on_resize(u32 sx, u32 sy) override
    {
        device.destroy_image(render_image);
        render_image = device.create_image({
            .format = daxa::Format::R8G8B8A8_UNORM,
            .size = {surface_width, surface_height, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        });
        task_render_image.set_images({.images = std::array{render_image}});
        camera.setViewportSize(sx, sy);
    }

    void record_tasks(daxa::TaskGraph &task_graph) override
    {
        task_graph.use_persistent_image(task_render_image);
        task_graph.use_persistent_buffer(task_chunk_occupancy_buffer);
        task_graph.use_persistent_buffer(task_brick_occupancy_buffer);
        task_graph.use_persistent_buffer(task_state_buffer);

        task_graph.add_task({
            .attachments =
                {
                    daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_render_image),
                    daxa::inl_attachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_chunk_occupancy_buffer),
                    daxa::inl_attachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_brick_occupancy_buffer),
                    daxa::inl_attachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_state_buffer),
                },
            .task = [this](daxa::TaskInterface ti) {
                stateData.camera = getCameraData(camera);
                this->allocate_fill_copy<StateData>(ti, stateData, ti.get(task_state_buffer), 0);

                ti.recorder.set_pipeline(*compute_pipeline);
                ti.recorder.push_constant(ComputePush{
                    .image = render_image.default_view(),
                    .chunk_occupancy_ptr = get_device_address(ti, task_chunk_occupancy_buffer, 0),
                    .brick_occupancy_ptr = get_device_address(ti, task_brick_occupancy_buffer, 0),
                    .state_ptr = get_device_address(ti, task_state_buffer, 0),
                    .frame_dim = {Application::surface_width, Application::surface_height},
                    .time = time,
                });
                ti.recorder.dispatch({(Application::surface_width + 7) / 8, (Application::surface_height + 7) / 8});
            },
            .name = APPNAME_PREFIX("Draw (Compute)"),
        });

        task_graph.add_task({
            .attachments = {
                {daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_READ, task_render_image)},
                {daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_WRITE, task_swapchain_image)},
            },
            .task = [this](daxa::TaskInterface task) {
                task.recorder.blit_image_to_image({
                    .src_image = task.get(task_render_image).ids[0],
                    .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                    .dst_image = task.get(task_swapchain_image).ids[0],
                    .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .src_offsets = {{{0, 0, 0},
                                     {(i32)(Application::surface_width),
                                      (i32)(Application::surface_height), 1}}},
                    .dst_offsets = {{{0, 0, 0},
                                     {(i32)(Application::surface_width),
                                      (i32)(Application::surface_height), 1}}},
                });
            },
            .name = APPNAME_PREFIX("Blit (render to swapchain)"),
        });
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
    template <typename T>
    inline void allocate_fill_copy(daxa::TaskInterface ti, T value, daxa::TaskBufferAttachmentInfo dst, u32 dst_offset = 0)
    {
        auto alloc = ti.allocator->allocate_fill(value).value();
        ti.recorder.copy_buffer_to_buffer({
            .src_buffer = ti.allocator->buffer(),
            .dst_buffer = dst.ids[0],
            .src_offset = alloc.buffer_offset,
            .dst_offset = dst_offset,
            .size = sizeof(T),
        });
    }

    inline DeviceAddress get_device_address(daxa::TaskInterface ti, daxa::TaskBuffer buffer, usize bufferIndex)
    {
        return ti.device.buffer_device_address(ti.get(buffer).ids[bufferIndex]).value();
    }

    inline void create_buffer(daxa::Device device, usize bytes, daxa::BufferId &out_buffer, daxa::TaskBuffer &out_task_buffer, std::string name)
    {
        out_buffer = device.create_buffer({
            .size = bytes,
            .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
        });

        out_task_buffer = daxa::TaskBuffer({.initial_buffers = {
                                                .buffers = std::array{out_buffer}},
                                            .name = name});
    }

    template <typename T>
    inline void create_buffer(daxa::Device device, daxa::BufferId &out_buffer, daxa::TaskBuffer &out_task_buffer, std::string name)
    {
        create_buffer(device, sizeof(T), out_buffer, out_task_buffer, name);
    }
};