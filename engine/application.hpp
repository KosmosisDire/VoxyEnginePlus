#pragma once

#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <thread>
#include <window.hpp>

#include <daxa/utils/imgui.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <fonts.hpp>
#include <imgui_impl_glfw.h>
#include <input.hpp>
#include <stdio.h>

using namespace std::chrono_literals;
using Clock = std::chrono::high_resolution_clock;

#define APPNAME "VoxyEngine"
#define APPNAME_PREFIX(x) ("[" APPNAME "] " x)

struct Application
{
  public:
    Application();
    virtual ~Application();

    // Prevent copying
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    bool update();
    virtual void on_update();
    virtual void record_tasks(daxa::TaskGraph &task_graph);

    void capture_mouse(bool capture);
    bool is_mouse_captured() { return mouseCaptured; }

  protected:
    std::unique_ptr<Window> window = std::make_unique<Window>(APPNAME);
    daxa::Instance daxa_ctx = daxa::create_instance({});
    daxa::Device device = [&]() {
        daxa::DeviceInfo2 info = {.name = "default device"};
        daxa::ImplicitFeatureFlags required_features = {};
        info = daxa_ctx.choose_device(required_features, info);
        return daxa_ctx.create_device_2(info);
    }();

    daxa::Swapchain swapchain = device.create_swapchain({.native_window = window->get_native_handle(),
                                                         .native_window_platform = window->get_native_platform(),
                                                         .present_mode = daxa::PresentMode::FIFO,
                                                         .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
                                                         .name = "swapchain"});

    daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
        .device = device,
        .shader_compile_options =
            {
                .root_paths = {DAXA_SHADER_INCLUDE_DIR, "src/shaders/"},
                .language = daxa::ShaderLanguage::SLANG,
                .enable_debug_info = true,
            },
        .name = "pipeline_manager",
    });

    // this must be created before the imgui renderer or the imgui renderer will not respond to input
    InputManager inputManager = InputManager(window->get_glfw_window());

    daxa::ImGuiRenderer imgui_renderer = create_imgui_renderer();

    u32 surface_width = 0;
    u32 surface_height = 0;

    Clock::time_point start, prev_time = Clock::now();
    f32 time = 0.0f;
    f32 delta_time = 1.0f;

    daxa::TaskImage task_swapchain_image = daxa::TaskImage{{.swapchain_image = true, .name = "swapchain_image"}};
    std::vector<daxa::TaskAttachmentInfo> imgui_task_attachments{};
    daxa::TaskGraph loop_task_graph;

    virtual void on_resize(u32 sx, u32 sy);

    void init();

  private:
    bool mouseCaptured = false;
    void base_on_update();
    void base_on_resize(u32 sx, u32 sy);
    daxa::ImGuiRenderer create_imgui_renderer();
    daxa::TaskGraph record_loop_task_graph();
};

inline Application::Application()
{
}

inline Application::~Application()
{
    device.wait_idle();
    device.collect_garbage();
    ImGui_ImplGlfw_Shutdown();
}

// this should be called from derived class constructor
inline void Application::init()
{
    loop_task_graph = record_loop_task_graph();
    base_on_resize((u32)window->get_width(), (u32)window->get_height());
}

inline bool Application::update()
{
    glfwPollEvents();
    if (glfwWindowShouldClose(window->get_glfw_window()))
    {
        return true;
    }

    if (!window->is_minimized())
    {
        base_on_update();
        InputManager::Update();
    }
    else
    {
        std::this_thread::sleep_for(1ms);
    }

    return false;
}

inline void Application::capture_mouse(bool capture)
{
    window->set_mouse_capture(capture);
    mouseCaptured = capture;

    InputManager::SetMousePosition(window->get_width() / 2, window->get_height() / 2);
    InputManager::ResetMouseDelta();
}

inline void Application::base_on_update()
{
    auto now = Clock::now();
    time = std::chrono::duration<f32>(now - start).count();
    delta_time = std::chrono::duration<f32>(now - prev_time).count();
    prev_time = now;

    auto reloaded_result = pipeline_manager.reload_all();
    if (auto reload_err =
            daxa::get_if<daxa::PipelineReloadError>(&reloaded_result))
        std::cout << "Failed to reload " << reload_err->message << '\n';
    if (daxa::get_if<daxa::PipelineReloadSuccess>(&reloaded_result))
        std::cout << "Successfully reloaded!\n";

    auto swapchain_image = swapchain.acquire_next_image();
    task_swapchain_image.set_images({.images = std::array{swapchain_image}});
    if (swapchain_image.is_empty())
    {
        std::cout << "Failed to acquire next image\n";
        return;
    }

    loop_task_graph.execute({});
    on_update();

    device.collect_garbage();
}

inline void Application::base_on_resize(u32 sx, u32 sy)
{
    window->set_minimized(sx == 0 || sy == 0);
    if (!window->is_minimized())
    {
        swapchain.resize();

        surface_width = swapchain.get_surface_extent().x;
        surface_height = swapchain.get_surface_extent().y;

        on_resize(sx, sy);

        std::cout << "Resized to " << sx << "x" << sy << '\n';

        base_on_update();
    }
}

inline daxa::ImGuiRenderer Application::create_imgui_renderer()
{
    auto ctx = ImGui::CreateContext();

    FontManager::LoadDefaults();

    ImGui_ImplGlfw_InitForVulkan(window->get_glfw_window(), true);
    return daxa::ImGuiRenderer(
        {.device = device, .format = swapchain.get_format(), .context = ctx});
}

inline daxa::TaskGraph Application::record_loop_task_graph()
{
    daxa::TaskGraph new_task_graph = daxa::TaskGraph({
        .device = device,
        .swapchain = swapchain,
        .use_split_barriers = false,
        .record_debug_information = true,
        .name = "main_task_graph",
    });

    new_task_graph.use_persistent_image(task_swapchain_image);

    record_tasks(new_task_graph);

    imgui_task_attachments.push_back(daxa::inl_attachment(
        daxa::TaskImageAccess::COLOR_ATTACHMENT, task_swapchain_image));
    auto imgui_task_info = daxa::InlineTaskInfo{
        .attachments = std::move(imgui_task_attachments),
        .task =
            [this](daxa::TaskInterface const &ti) {
                imgui_renderer.record_commands(ImGui::GetDrawData(), ti.recorder,
                                               ti.get(task_swapchain_image).ids[0],
                                               surface_width, surface_height);
            },
        .name = "ImGui Task",
    };

    new_task_graph.add_task(imgui_task_info);

    new_task_graph.submit({});
    new_task_graph.present({});
    new_task_graph.complete({});

    return new_task_graph;
}

// Virtual function implementations with default behavior
inline void Application::on_update() {}

inline void Application::on_resize(u32 sx, u32 sy) {}

inline void Application::record_tasks(daxa::TaskGraph &task_graph) {}