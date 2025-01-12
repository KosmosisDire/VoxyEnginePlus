#pragma once
#include <daxa/daxa.hpp>
using namespace daxa::types;
#include "terminal-colors.hpp"
#include "window.hpp"
#include <daxa/utils/imgui.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <functional>
#include <imgui_impl_glfw.h>
#include <string>
#include <vector>

struct InlineTask
{
    std::vector<daxa::TaskAttachmentInfo> attachments = {};
    std::function<void(daxa::TaskInterface)> task = {};
    std::string_view name = "unnamed";

    InlineTask(std::string_view name)
        : name(name)
    {
    }

    // make builder interface
    InlineTask &add_attachment(daxa::TaskAttachmentInfo attachment)
    {
        attachments.push_back(attachment);
        return *this;
    }

    InlineTask &set_task(std::function<void(daxa::TaskInterface)> task)
    {
        this->task = task;
        return *this;
    }

  private:
    friend class Renderer;
    daxa::InlineTaskInfo build()
    {
        return {.attachments = attachments, .task = task, .name = name};
    }
};

class Renderer
{
  public:
    u32 surface_width = 0;
    u32 surface_height = 0;
    daxa::Instance daxa_instance;
    daxa::Device device;
    daxa::Swapchain swapchain;
    daxa::PipelineManager pipeline_manager;
    daxa::TaskImage task_swap_image;
    daxa::ImGuiRenderer imgui_renderer;
    daxa::TaskGraph render_loop_graph;
    bool task_graph_complete = false;

    /// @brief The renderer sets up a simple render loop which can be extended by adding tasks to the pipeline.
    /// @param window
    /// @param shaderDirectories Directories containing shaders. Those shaders can then be referenced via a relative path from the shader directory.
    Renderer(Window *window, std::vector<std::filesystem::path> shaderDirectories)
        : window(window)
    {
        daxa_instance = daxa::create_instance({});
        device = daxa_instance.create_device_2(daxa_instance.choose_device({}, {}));
        swapchain = CreateSwapchain(device, window, "swapchain");
        task_swap_image = CreateSwapchainImage(swapchain, "swapchain_image");
        pipeline_manager = CreatePipelineManager(device, shaderDirectories, "pipeline_manager");
        imgui_renderer = CreateImguiRenderer();

        Resize(window->GetWidth(), window->GetHeight());
        NewFrame();
        InitializeGraph();
    }

    ~Renderer()
    {
        device.wait_idle();
        device.collect_garbage();
        ImGui_ImplGlfw_Shutdown();
    }

    void Update()
    {
        if (!task_graph_complete)
        {
            printf(RED "Task graph not completed before frame start!!\n" RESET);
            Complete();
        }

        NewFrame();

        render_loop_graph.execute({});

        device.collect_garbage();
    }

    void Resize(u32 width, u32 height)
    {
        swapchain.resize();
        surface_width = swapchain.get_surface_extent().x;
        surface_height = swapchain.get_surface_extent().y;
    }

    inline void Complete()
    {

        auto imgui_task_info = daxa::InlineTaskInfo{
            .attachments = {daxa::inl_attachment(daxa::TaskImageAccess::COLOR_ATTACHMENT, task_swap_image)},
            .task = [this](daxa::TaskInterface const &ti)
            {
                imgui_renderer.record_commands(ImGui::GetDrawData(), ti.recorder, ti.get(task_swap_image).ids[0], surface_width, surface_height);
            },
            .name = "ImGui Task",
        };

        render_loop_graph.add_task(imgui_task_info);

        render_loop_graph.submit({});
        render_loop_graph.present({});
        render_loop_graph.complete({});
        task_graph_complete = true;
    }

    /// @brief Add a task to the render loop
    /// @param task The task to add. This can be a new insteance of InlineTask.
    /// @param defer_completion If true, the task graph will not be rebuilt (but will be reinitialized). This only matters if the graph was already completed before this task was added.
    inline void AddTask(InlineTask task, bool defer_completion = false)
    {
        if (task_graph_complete)
        {
            InitializeGraph();
        }

        render_loop_graph.add_task(task.build());

        if (!defer_completion && task_graph_complete)
        {
            Complete();
        }
    }

    inline void AddTask(daxa::InlineTaskInfo task, bool defer_completion = false)
    {
        if (task_graph_complete)
        {
            InitializeGraph();
        }

        render_loop_graph.add_task(task);

        if (!defer_completion && task_graph_complete)
        {
            Complete();
        }
    }

    inline void DestroyImage(daxa::ImageId image)
    {
        device.wait_idle();
        device.destroy_image(image);
    }

    inline void DestroyBuffer(daxa::BufferId buffer)
    {
        device.wait_idle();
        device.destroy_buffer(buffer);
    }

    inline daxa::ImageId CreateImage(daxa::ImageInfo info)
    {
        return device.create_image(info);
    }

    inline daxa::ImageId CreateRenderImage(std::string name, daxa::ImageUsageFlags flags = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC)
    {
        return CreateImage({
            .format = swapchain.get_format(),
            .size = {surface_width, surface_height, 1},
            .usage = flags,
            .name = name,
        });
    }

    inline daxa::ImageId CreateImage(daxa::ImageInfo info, daxa::TaskImage &out_task_image)
    {
        auto id = CreateImage(info);
        out_task_image = daxa::TaskImage({.initial_images = {.images = std::array{id}}, .name = ("task_" + std::string(info.name.view())).c_str()});
        return id;
    }

    inline daxa::ImageId CreateRenderImage(std::string name, daxa::TaskImage &out_task_image, daxa::ImageUsageFlags flags = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC)
    {
        auto id = CreateRenderImage(name, flags);
        out_task_image = daxa::TaskImage({.initial_images = {.images = std::array{id}}, .name = ("task_" + name).c_str()});
        return id;
    }

    template <typename PushConstant>
    inline std::shared_ptr<daxa::ComputePipeline> AddComputePipeline(std::string name, std::filesystem::path shaderPath)
    {
        return pipeline_manager.add_compute_pipeline({
                                                         .shader_info = {.source = daxa::ShaderFile{shaderPath}},
                                                         .push_constant_size = sizeof(PushConstant),
                                                         .name = name,
                                                     })
            .value();
    }

    inline void CreateBuffer(std::string name, usize bytes, daxa::BufferId &out_buffer, daxa::TaskBuffer &out_task_buffer)
    {
        out_buffer = device.create_buffer({
            .size = bytes,
            .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
        });

        out_task_buffer = daxa::TaskBuffer({.initial_buffers = {.buffers = std::array{out_buffer}}, .name = ("task_" + name).c_str()});
    }

    template <typename T>
    inline void CreateBuffer(std::string name, daxa::BufferId &out_buffer, daxa::TaskBuffer &out_task_buffer)
    {
        CreateBuffer(name, sizeof(T), out_buffer, out_task_buffer);
    }

    template <typename T>
    inline T *MapBufferAs(daxa::BufferId buffer)
    {
        return device.get_host_address_as<T>(buffer).value();
    }

    inline daxa::InlineTaskInfo CreateBlitTask(daxa::TaskImage src, daxa::TaskImage dst)
    {
        return {
            .attachments = {
                {daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_READ, src)},
                {daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_WRITE, dst)},
            },
            .task = [src, dst, this](daxa::TaskInterface task)
            {
                task.recorder.blit_image_to_image({
                    .src_image = task.get(src).ids[0],
                    .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                    .dst_image = task.get(dst).ids[0],
                    .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .src_offsets = {{{0, 0, 0}, {(i32)(surface_width), (i32)(surface_height), 1}}},
                    .dst_offsets = {{{0, 0, 0}, {(i32)(surface_width), (i32)(surface_height), 1}}},
                });
            },
            .name = "Blit " + std::string(src.info().name) + " -> " + std::string(dst.info().name),
        };
    }

    inline daxa::InlineTaskInfo CreateSwapchainBlitTask(daxa::TaskImage src)
    {
        return CreateBlitTask(src, task_swap_image);
    }

  private:
    Window *window;

    void NewFrame()
    {
        auto reloaded_result = pipeline_manager.reload_all();
        if (auto reload_err =
                daxa::get_if<daxa::PipelineReloadError>(&reloaded_result))
            std::cout << "Failed to reload " << reload_err->message << '\n';
        if (daxa::get_if<daxa::PipelineReloadSuccess>(&reloaded_result))
            std::cout << "Successfully reloaded!\n";

        auto swapchain_image = swapchain.acquire_next_image();
        task_swap_image.set_images({.images = std::array{swapchain_image}});
        if (swapchain_image.is_empty())
        {
            std::cout << "Failed to acquire next image\n";
            return;
        }
    }

    void InitializeGraph()
    {
        task_graph_complete = false;
        render_loop_graph = daxa::TaskGraph({
            .device = device,
            .swapchain = swapchain,
            .use_split_barriers = false,
            .record_debug_information = true,
            .name = "render_loop",
        });

        render_loop_graph.use_persistent_image(task_swap_image);
    }

    inline daxa::ImGuiRenderer CreateImguiRenderer()
    {
        auto ctx = ImGui::CreateContext();

        FontManager::LoadDefaults();

        ImGui_ImplGlfw_InitForVulkan(window->GetGlfwWindow(), true);
        return daxa::ImGuiRenderer(
            {.device = device,
             .format = swapchain.get_format(),
             .context = ctx}
        );
    }

  public:
    template <typename T>
    static inline void CopyToBuffer(daxa::TaskInterface ti, T src, daxa::TaskBufferAttachmentInfo dst, u32 dst_offset = 0)
    {
        auto alloc = ti.allocator->allocate_fill(src).value();
        ti.recorder.copy_buffer_to_buffer({
            .src_buffer = ti.allocator->buffer(),
            .dst_buffer = dst.ids[0],
            .src_offset = alloc.buffer_offset,
            .dst_offset = dst_offset,
            .size = sizeof(T),
        });
    }

    static inline DeviceAddress GetDeviceAddress(daxa::TaskInterface ti, daxa::TaskBuffer buffer, usize bufferIndex)
    {
        return ti.device.buffer_device_address(ti.get(buffer).ids[bufferIndex]).value();
    }

    static inline daxa::Swapchain CreateSwapchain(daxa::Device &device, Window *window, std::string name)
    {
        return device.create_swapchain({.native_window = window->GetNativeHandle(), .native_window_platform = window->GetNativePlatform(), .present_mode = daxa::PresentMode::MAILBOX, .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST, .name = name});
    }

    static inline daxa::TaskImage CreateSwapchainImage(daxa::Swapchain &swapchain, std::string name)
    {
        return daxa::TaskImage{{.swapchain_image = true, .name = name}};
    }

    static inline daxa::PipelineManager CreatePipelineManager(daxa::Device &device, std::vector<std::filesystem::path> shaderDirectories, std::string name)
    {
        shaderDirectories.push_back(DAXA_SHADER_INCLUDE_DIR);
        return daxa::PipelineManager({
            .device = device,
            .shader_compile_options = {
                .root_paths = shaderDirectories,
                .language = daxa::ShaderLanguage::SLANG,
                .enable_debug_info = true,
            },
            .name = name,
        });
    }
};