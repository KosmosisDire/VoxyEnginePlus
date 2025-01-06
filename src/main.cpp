#include "application.hpp"
#include "stdio.h"

using namespace daxa::types;
#include "shaders/shared.inl"

struct VoxyApp : public Application
{
  public:
    Settings settings = {
        .backgroundColor = {0.0f, 0.0f, 0.0f, 1.0f},
    };

    VoxyApp()
    {
        // Initialize compute pipeline
        compute_pipeline = pipeline_manager.add_compute_pipeline({
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
        occupancy_buffer = device.create_buffer({
            .size = (64 * 64 * 64) / sizeof(u64), // 64x64x64 voxels, 1 bit per voxel. 64 bit integer = 8 bytes per integer
            .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
            .name = "occupancy_buffer",
        });

        task_occupancy_buffer = daxa::TaskBuffer({.initial_buffers = {
                                                      .buffers = std::array{occupancy_buffer}},
                                                  .name = "task_occupancy_buffer"});

        // write every other bit to 1
        auto *ptr = device.buffer_host_address_as<Occupancy>(occupancy_buffer).value();

        for (u64 i = 0; i < (64 * 64 * 64); i++)
        {
            ptr->occupancy[i] = 0xAAAAAAAAAAAAAAAA;
        }

        Application::init();
    }

    ~VoxyApp()
    {
        device.wait_idle();
        device.collect_garbage();
        device.destroy_image(render_image);
        device.destroy_buffer(occupancy_buffer);
    }

  protected:
    void on_update() override
    {
        ui_update();
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
    }

    void record_tasks(daxa::TaskGraph &task_graph) override
    {
        std::cout << "Recording tasks\n";
        task_graph.use_persistent_image(task_render_image);
        task_graph.use_persistent_buffer(task_occupancy_buffer);

        task_graph.add_task({
            .attachments =
                {
                    daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_render_image),
                    daxa::inl_attachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_occupancy_buffer),
                },
            .task = [this](daxa::TaskInterface ti) {
                ti.recorder.set_pipeline(*compute_pipeline);
                ti.recorder.push_constant(ComputePush{
                    .settings = settings,
                    .image = render_image.default_view(),
                    .occupancy = device.buffer_device_address(ti.get(task_occupancy_buffer).ids[0]).value(),
                    .frame_dim = {Application::surface_width, Application::surface_height},
                    .time = time});
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
    void ui_update()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        float color[4] = {settings.backgroundColor.x, settings.backgroundColor.y, settings.backgroundColor.z, 1.0f};
        ImGui::Begin("Settings");
        ImGui::ColorEdit4("Background Color", color);
        settings.backgroundColor = {color[0], color[1], color[2], 1.0f};
        ImGui::End();

        ImGui::Render();
    }

    std::shared_ptr<daxa::ComputePipeline> compute_pipeline;
    daxa::ImageId render_image;
    daxa::TaskImage task_render_image;
    daxa::BufferId occupancy_buffer;
    daxa::TaskBuffer task_occupancy_buffer;
};

auto main() -> int
{
    VoxyApp app;
    while (true)
    {
        if (app.update())
        {
            break;
        }
    }
    return 0;
}