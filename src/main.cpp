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

        Application::init();
    }

    ~VoxyApp()
    {
        device.wait_idle();
        device.collect_garbage();
        device.destroy_image(render_image);
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

    void record_tasks(daxa::TaskGraph &new_task_graph) override
    {
        std::cout << "Recording tasks\n";
        new_task_graph.use_persistent_image(task_render_image);

        new_task_graph.add_task({
            .attachments = {daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_render_image)},
            .task = [this](daxa::TaskInterface ti) {
                ti.recorder.set_pipeline(*compute_pipeline);
                ti.recorder.push_constant(ComputePush{
                    .settings = settings,
                    .image = render_image.default_view(),
                    .frame_dim = {Application::surface_width, Application::surface_height},
                    .time = time});
                ti.recorder.dispatch({(Application::surface_width + 7) / 8, (Application::surface_height + 7) / 8});
            },
            .name = APPNAME_PREFIX("Draw (Compute)"),
        });

        new_task_graph.add_task({
            .attachments = {
                {daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_READ, task_render_image)},
                {daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_WRITE, task_swapchain_image)},
            },
            .task = [this](daxa::TaskInterface ti) {
                ti.recorder.blit_image_to_image({
                    .src_image = ti.get(task_render_image).ids[0],
                    .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                    .dst_image = ti.get(task_swapchain_image).ids[0],
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