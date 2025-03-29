#pragma once
#include <daxa/daxa.hpp>
using namespace daxa::types;
#include <engine/data/Image.hpp>
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
        InlineTask &AddAttachment(daxa::TaskAttachmentInfo attachment)
        {
            attachments.push_back(attachment);
            return *this;
        }

        InlineTask &AddAttachment(daxa::TaskImageAccess access, daxa::TaskImageView view)
        {
            attachments.push_back(daxa::inl_attachment(access, view));
            return *this;
        }

        InlineTask &AddAttachment(daxa::TaskBufferAccess access, daxa::TaskBuffer buffer)
        {
            attachments.push_back(daxa::inl_attachment(access, buffer));
            return *this;
        }

        InlineTask &SetTask(std::function<void(daxa::TaskInterface)> task)
        {
            this->task = task;
            return *this;
        }

        InlineTask &AddAllAttachments(daxa::TaskImageAccess access, std::vector<daxa::TaskImage> images)
        {
            for (auto image : images)
            {
                AddAttachment(access, image);
            }

            return *this;
        }

        InlineTask &AddAllAttachments(daxa::TaskBufferAccess access, std::vector<daxa::TaskBuffer> buffers)
        {
            for (auto buffer : buffers)
            {
                AddAttachment(access, buffer);
            }

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
        Renderer(std::shared_ptr<GameWindow> window, std::vector<std::filesystem::path> shaderDirectories)
            : window(window)
        {
            daxa_instance = daxa::create_instance({});
            device = daxa_instance.create_device_2(daxa_instance.choose_device(daxa::ImplicitFeatureFlagBits::SHADER_FLOAT16, {}));
            swapchain = CreateSwapchain(device, window, "swapchain");
            task_swap_image = CreateSwapchainImage(swapchain, "swapchain_image");
            pipeline_manager = CreatePipelineManager(device, shaderDirectories, "pipeline_manager");
            imgui_renderer = CreateImguiRenderer();

            Resize(window->GetWidth(), window->GetHeight());
            InitializeGraph();
        }

        ~Renderer()
        {
            device.wait_idle();
            device.collect_garbage();
            ImGui_ImplGlfw_Shutdown();
        }

        void Render()
        {
            if (!task_graph_complete)
            {
                printf("Task graph not completed before frame start!!\n");
                Complete();
            }

            auto reloaded_result = pipeline_manager.reload_all();

            if (auto reload_err = daxa::get_if<daxa::PipelineReloadError>(&reloaded_result))
            {
                std::cout << "Failed to reload " << reload_err->message << '\n';
            }

            if (daxa::get_if<daxa::PipelineReloadSuccess>(&reloaded_result))
            {
                std::cout << "Successfully reloaded!\n";

            }

            auto swapchain_image = swapchain.acquire_next_image();
            task_swap_image.set_images({.images = std::array{swapchain_image}});

            if (swapchain_image.is_empty())

            {
                std::cout << "Failed to acquire next image\n";
                return;
            }

            render_loop_graph.execute({});
        }

        void Resize(u32 width, u32 height)
        {
            swapchain.resize();
            surface_width = swapchain.get_surface_extent().x;
            surface_height = swapchain.get_surface_extent().y;
        }

        inline void Complete()
        {
            auto imgui_task_info = daxa::InlineTaskInfo
            {
                .attachments = {
                    daxa::inl_attachment(daxa::TaskImageAccess::COLOR_ATTACHMENT, task_swap_image)
                },
                .task = [this](daxa::TaskInterface const & ti)
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
            AddTask(task.build(), defer_completion);
        }

        /// @brief Add a task to the render loop
        /// @param task The task to add as an InlineTaskInfo.
        /// @param defer_completion If true, the task graph will not be rebuilt (but will be reinitialized). This only matters if the graph was already completed before this task was added.
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
            device.destroy_image(image);
        }

        inline void DestroyBuffer(daxa::BufferId buffer)
        {
            device.destroy_buffer(buffer);
        }

        inline daxa::ImageId CreateImage(daxa::ImageInfo info)
        {
            return device.create_image(info);
        }

        inline daxa::ImageId CreateImage(std::string name, Image &image)
        {
            auto id = CreateImage(daxa::ImageInfo
            {
                .format = daxa::Format::R8G8B8A8_UNORM,
                .size = {(unsigned int)image.width(), (unsigned int)image.height(), 1},
                .usage = daxa::ImageUsageFlagBits::TRANSFER_DST | daxa::ImageUsageFlagBits::SHADER_STORAGE,
                .name = name,
            });

            // create temp buffer
            auto buffer = device.create_buffer(
            {
                .size = image.size(),
                .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
            });

            // copy image data to buffer
            auto data = device.buffer_host_address_as<std::byte>(buffer).value();
            std::memcpy(data, image.data(), image.size());

            // execute a task to copy the image data to the gpu
            auto recorder = device.create_command_recorder({});

            recorder.pipeline_barrier_image_transition(
            {
                .src_access = {
                    .stages = daxa::PipelineStageFlagBits::NONE,
                    .type = daxa::AccessTypeFlagBits::READ
                },
                .dst_access = {
                    .stages = daxa::PipelineStageFlagBits::TRANSFER,
                    .type = daxa::AccessTypeFlagBits::WRITE
                },
                .src_layout = daxa::ImageLayout::UNDEFINED,
                .dst_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                .image_id = id,
            });

            recorder.copy_buffer_to_image(
            {
                .buffer = buffer,
                .image = id,
                .image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                .image_slice = {0, 0},
                .image_offset = {0, 0, 0},
                .image_extent = {(unsigned int)image.width(), (unsigned int)image.height(), 1},
            });

            recorder.pipeline_barrier_image_transition(
            {
                .src_access = {
                    .stages = daxa::PipelineStageFlagBits::TRANSFER,
                    .type = daxa::AccessTypeFlagBits::WRITE
                },
                .dst_access = {
                    .stages = daxa::PipelineStageFlagBits::NONE,
                    .type = daxa::AccessTypeFlagBits::READ
                },
                .src_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                .dst_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
                .image_id = id,
            });

            auto executable_commands = recorder.complete_current_commands();
            recorder.~CommandRecorder();
            device.submit_commands({.command_lists = std::array{executable_commands}});
            device.wait_idle();
            device.collect_garbage();

            device.destroy_buffer(buffer);

            return id;
        }

        static constexpr daxa::ImageUsageFlags color_image_flags = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST;
        static constexpr daxa::ImageUsageFlags depth_image_flags = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST;
        static constexpr daxa::ImageUsageFlags transfer_image_flags = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST;

        inline daxa::ImageId CreateRenderImage(std::string name, daxa::ImageUsageFlags flags = color_image_flags)
        {
            return CreateImage(
            {
                .format = swapchain.get_format(),
                .size = {surface_width, surface_height, 1},
                .usage = flags,
                .name = name,
            });
        }

        inline daxa::ImageId CreateRenderImage(std::string name, daxa::Format format, float scale = 1.0, daxa::ImageUsageFlags flags = color_image_flags)
        {
            return CreateImage(
            {
                .format = format,
                .size = {static_cast<u32>(surface_width * scale), static_cast<u32>(surface_height * scale), 1},
                .usage = flags,
                .name = name,
            });
        }

        inline void SetTaskImage(std::string name, daxa::ImageId image, daxa::TaskImage *out_task_image)
        {
            if (out_task_image->is_valid())
            {
                out_task_image->set_images({.images = std::array{image}});
            }
            else
            {
                *out_task_image = daxa::TaskImage({.initial_images = {.images = std::array{image}}, .name = ("task_" + name).c_str()});
            }
        }

        inline daxa::ImageId CreateRenderImage(std::string name, daxa::TaskImage *out_task_image, daxa::Format format, float scale = 1.0, daxa::ImageUsageFlags flags = color_image_flags)
        {
            auto id = CreateRenderImage(name, format, scale, flags);
            SetTaskImage(name, id, out_task_image);
            return id;
        }

        inline daxa::ImageId CreateImage(daxa::ImageInfo info, daxa::TaskImage *out_task_image)
        {
            auto id = CreateImage(info);
            SetTaskImage(std::string(info.name.view()), id, out_task_image);
            return id;
        }

        inline daxa::ImageId CreateRenderImage(std::string name, daxa::TaskImage *out_task_image, daxa::ImageUsageFlags flags = color_image_flags)
        {
            auto id = CreateRenderImage(name, flags);
            SetTaskImage(name, id, out_task_image);
            return id;
        }

        template <typename PushConstant>
        inline std::shared_ptr<daxa::ComputePipeline> AddComputePipeline(std::string name, std::filesystem::path shaderPath)
        {
            return pipeline_manager.add_compute_pipeline(
            {
                .shader_info = {.source = daxa::ShaderFile{shaderPath}},
                .push_constant_size = sizeof(PushConstant),
                .name = name,
            }).value();
        }


        inline void CreateBuffer(std::string name, usize bytes, daxa::BufferId & out_buffer, daxa::TaskBuffer & out_task_buffer, daxa::MemoryFlags flags = daxa::MemoryFlagBits::NONE)
        {
            out_buffer = device.create_buffer(
            {
                .size = bytes,
                .allocate_info = flags,
            });

            out_task_buffer = daxa::TaskBuffer({.initial_buffers = {.buffers = std::array{out_buffer}}, .name = ("task_" + name).c_str()});
        }

        template <typename T>
        inline void CreateBuffer(std::string name, daxa::BufferId & out_buffer, daxa::TaskBuffer & out_task_buffer, daxa::MemoryFlags flags = daxa::MemoryFlagBits::NONE)
        {
            CreateBuffer(name, sizeof(T), out_buffer, out_task_buffer, flags);
        }

        template <typename T>
        inline T *MapBufferAs(daxa::BufferId buffer)
        {
            return device.buffer_host_address_as<T>(buffer).value();
        }

        inline daxa::InlineTaskInfo CreateBlitTask(daxa::TaskImage src, daxa::TaskImage dst)
        {
            return
            {
                .attachments = {
                    {daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_READ, src)},
                    {daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_WRITE, dst)},
                },
                .task = [src, dst, this](daxa::TaskInterface task)
                {
                    task.recorder.blit_image_to_image(
                    {
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
        std::shared_ptr<GameWindow> window;

        void InitializeGraph()
        {
            task_graph_complete = false;
            render_loop_graph = daxa::TaskGraph(
            {
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
            {
                .device = device,
                .format = swapchain.get_format(),
                .context = ctx
            });
        }

    public:
        template <typename T>
        static inline void CopyToBuffer(daxa::TaskInterface ti, T src, daxa::TaskBufferAttachmentInfo dst, u32 dst_offset = 0)
        {
            auto alloc = ti.allocator->allocate_fill(src).value();
            ti.recorder.copy_buffer_to_buffer(
            {
                .src_buffer = ti.allocator->buffer(),
                .dst_buffer = dst.ids[0],
                .src_offset = alloc.buffer_offset,
                .dst_offset = dst_offset,
                .size = sizeof(T),
            });
        }

        static inline void CopyBuffer(daxa::TaskInterface ti, daxa::TaskBuffer src, daxa::TaskBuffer dst, u32 dst_offset = 0)
        {
            ti.recorder.copy_buffer_to_buffer(
            {
                .src_buffer = ti.get(src).ids[0],
                .dst_buffer = ti.get(dst).ids[0],
                .src_offset = 0,
                .dst_offset = dst_offset,
                .size = ti.device.buffer_info(ti.get(src).ids[0]).value().size,
            });
        }

        static inline void ClearBuffer(daxa::TaskInterface ti, daxa::TaskBufferAttachmentInfo buffer, u32 value = 0)
        {
            ti.recorder.clear_buffer(
            {
                .buffer = buffer.ids[0],
                .offset = 0,
                .size = ti.device.buffer_info(buffer.ids[0]).value().size,
                .clear_value = value
            });
        }

        static inline void ClearBuffer(daxa::TaskInterface ti, daxa::TaskBuffer buffer, u32 value = 0)
        {
            ClearBuffer(ti, ti.get(buffer), value);
        }

        static inline void CopyImage(daxa::TaskInterface ti, daxa::TaskImage src, daxa::TaskImage dst)
        {
            ti.recorder.copy_image_to_image({.src_image = ti.get(src).ids[0],
                                             .dst_image = ti.get(dst).ids[0]});
        }

        static inline DeviceAddress GetDeviceAddress(daxa::TaskInterface ti, daxa::TaskBuffer buffer, usize bufferIndex = 0)
        {
            return ti.device.buffer_device_address(ti.get(buffer).ids[bufferIndex]).value();
        }

        static inline daxa::Swapchain CreateSwapchain(daxa::Device & device, std::shared_ptr<GameWindow> window, std::string name)
        {
            auto native_handle = window->GetNativeHandle();
            auto platform = window->GetNativePlatform();

            std::cout << "Creating swapchain with:" << std::endl;
            std::cout << "  Platform: " << static_cast<int>(platform) << std::endl;
            std::cout << "  Handle: " << native_handle << std::endl;

            // Ensure we're not getting a null handle
            if (native_handle == nullptr)
            {
                throw std::runtime_error("Failed to get native window handle for swapchain creation");
            }

            return device.create_swapchain({.native_window = native_handle,
                                            .native_window_platform = platform,
                                            .present_mode = daxa::PresentMode::IMMEDIATE,
                                            .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
                                            .name = name});
        }

        static inline daxa::TaskImage CreateSwapchainImage(daxa::Swapchain & swapchain, std::string name)
        {
            return daxa::TaskImage{{.swapchain_image = true, .name = name}};
        }

        static inline daxa::PipelineManager CreatePipelineManager(daxa::Device & device, std::vector<std::filesystem::path> shaderDirectories, std::string name)
        {
            shaderDirectories.push_back(DAXA_SHADER_INCLUDE_DIR);
            return daxa::PipelineManager(
            {
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