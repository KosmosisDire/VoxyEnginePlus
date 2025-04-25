#pragma once
#include <daxa/daxa.hpp>
using namespace daxa::types;
#include <engine/data/Image.hpp>
#include "Window.hpp"
#include <engine/apis/Fonts.hpp> // Add include for FontManager
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

        InlineTask &add_attachment(daxa::TaskImageAccess access, daxa::TaskImageView view)
        {
            attachments.push_back(daxa::inl_attachment(access, view));
            return *this;
        }

        InlineTask &add_attachment(daxa::TaskBufferAccess access, daxa::TaskBuffer buffer)
        {
            attachments.push_back(daxa::inl_attachment(access, buffer));
            return *this;
        }

        InlineTask &set_task(std::function<void(daxa::TaskInterface)> task)
        {
            this->task = task;
            return *this;
        }

        InlineTask &add_all_attachments(daxa::TaskImageAccess access, std::vector<daxa::TaskImage> images)
        {
            for (auto image : images)
            {
                add_attachment(access, image); // Update call
            }

            return *this;
        }

        InlineTask &add_all_attachments(daxa::TaskBufferAccess access, std::vector<daxa::TaskBuffer> buffers)
        {
            for (auto buffer : buffers)
            {
                add_attachment(access, buffer); // Update call
            }

            return *this;
        }

    private:
        friend class Renderer;
        daxa::InlineTaskInfo build() // Keep build as is, it's private and called specifically
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
            swapchain = create_swapchain(device, window, "swapchain"); // Update call
            task_swap_image = create_swapchain_image(swapchain, "swapchain_image"); // Update call
            pipeline_manager = create_pipeline_manager(device, shaderDirectories, "pipeline_manager"); // Update call
            imgui_renderer = create_imgui_renderer(); // Update call

            resize(window->get_width(), window->get_height()); // Update call
            initialize_graph(); // Update call
        }

        ~Renderer()
        {
            device.wait_idle();
            device.collect_garbage();
            ImGui_ImplGlfw_Shutdown();
        }

        void render()
        {
            if (!task_graph_complete)
            {
                printf("Task graph not completed before frame start!!\n");
                complete(); // Update call
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

        void resize(u32 width, u32 height)
        {
            swapchain.resize();
            surface_width = swapchain.get_surface_extent().x;
            surface_height = swapchain.get_surface_extent().y;
        }

        inline void complete()
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
        inline void add_task(InlineTask task, bool defer_completion = false)
        {
            add_task(task.build(), defer_completion); // Update recursive call
        }

        /// @brief Add a task to the render loop
        /// @param task The task to add as an InlineTaskInfo.
        /// @param defer_completion If true, the task graph will not be rebuilt (but will be reinitialized). This only matters if the graph was already completed before this task was added.
        inline void add_task(daxa::InlineTaskInfo task, bool defer_completion = false)
        {
            if (task_graph_complete)
            {
                initialize_graph(); // Update call
            }

            render_loop_graph.add_task(task);

            if (!defer_completion && task_graph_complete)
            {
                complete(); // Update call
            }
        }

        inline void destroy_image(daxa::ImageId image)
        {
            device.destroy_image(image);
        }

        inline void destroy_buffer(daxa::BufferId buffer)
        {
            device.destroy_buffer(buffer);
        }

        inline daxa::ImageId create_image(daxa::ImageInfo info)
        {
            return device.create_image(info);
        }

        inline daxa::ImageId create_image(std::string name, Image &image)
        {
            auto id = create_image(daxa::ImageInfo // Update call
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

            destroy_buffer(buffer); // Update call

            return id;
        }

        static constexpr daxa::ImageUsageFlags color_image_flags = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST;
        static constexpr daxa::ImageUsageFlags depth_image_flags = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST;
        static constexpr daxa::ImageUsageFlags transfer_image_flags = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST;

        inline daxa::ImageId create_render_image(std::string name, daxa::ImageUsageFlags flags = color_image_flags)
        {
            return create_image( // Update call
            {
                .format = swapchain.get_format(),
                .size = {surface_width, surface_height, 1},
                .usage = flags,
                .name = name,
            });
        }

        inline daxa::ImageId create_render_image(std::string name, daxa::Format format, float scale = 1.0, daxa::ImageUsageFlags flags = color_image_flags)
        {
            return create_image( // Update call
            {
                .format = format,
                .size = {static_cast<u32>(surface_width * scale), static_cast<u32>(surface_height * scale), 1},
                .usage = flags,
                .name = name,
            });
        }

        inline void set_task_image(std::string name, daxa::ImageId image, daxa::TaskImage *out_task_image)
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

        inline daxa::ImageId create_render_image(std::string name, daxa::TaskImage *out_task_image, daxa::Format format, float scale = 1.0, daxa::ImageUsageFlags flags = color_image_flags)
        {
            auto id = create_render_image(name, format, scale, flags); // Update call
            set_task_image(name, id, out_task_image); // Update call
            return id;
        }

        inline daxa::ImageId create_image(daxa::ImageInfo info, daxa::TaskImage *out_task_image)
        {
            auto id = create_image(info); // Update call
            set_task_image(std::string(info.name.view()), id, out_task_image); // Update call
            return id;
        }

        inline daxa::ImageId create_render_image(std::string name, daxa::TaskImage *out_task_image, daxa::ImageUsageFlags flags = color_image_flags)
        {
            auto id = create_render_image(name, flags); // Update call
            set_task_image(name, id, out_task_image); // Update call
            return id;
        }

        template <typename PushConstant>
        inline std::shared_ptr<daxa::ComputePipeline> add_compute_pipeline(std::string name, std::filesystem::path shaderPath)
        {
            return pipeline_manager.add_compute_pipeline(
            {
                .shader_info = {.source = daxa::ShaderFile{shaderPath}},
                .push_constant_size = sizeof(PushConstant),
                .name = name,
            }).value();
        }


        inline void create_buffer(std::string name, usize bytes, daxa::BufferId & out_buffer, daxa::TaskBuffer & out_task_buffer, daxa::MemoryFlags flags = daxa::MemoryFlagBits::NONE)
        {
            out_buffer = device.create_buffer(
            {
                .size = bytes,
                .allocate_info = flags,
            });

            out_task_buffer = daxa::TaskBuffer({.initial_buffers = {.buffers = std::array{out_buffer}}, .name = ("task_" + name).c_str()});
        }

        template <typename T>
        inline void create_buffer(std::string name, daxa::BufferId & out_buffer, daxa::TaskBuffer & out_task_buffer, daxa::MemoryFlags flags = daxa::MemoryFlagBits::NONE)
        {
            create_buffer(name, sizeof(T), out_buffer, out_task_buffer, flags); // Update call
        }

        template <typename T>
        inline T *map_buffer_as(daxa::BufferId buffer)
        {
            return device.buffer_host_address_as<T>(buffer).value();
        }

        inline daxa::InlineTaskInfo create_blit_task(daxa::TaskImage src, daxa::TaskImage dst)
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

        inline daxa::InlineTaskInfo create_swapchain_blit_task(daxa::TaskImage src)
        {
            return create_blit_task(src, task_swap_image); // Update call
        }

    private:
        std::shared_ptr<GameWindow> window;

        void initialize_graph()
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

        inline daxa::ImGuiRenderer create_imgui_renderer()
        {
            auto ctx = ImGui::CreateContext();

            FontManager::load_defaults(); // Update call

            ImGui_ImplGlfw_InitForVulkan(window->get_glfw_window(), true); // Fix call
            return daxa::ImGuiRenderer(
            {
                .device = device,
                .format = swapchain.get_format(),
                .context = ctx
            });
        }

    public:
        template <typename T>
        static inline void copy_to_buffer(daxa::TaskInterface ti, T src, daxa::TaskBufferAttachmentInfo dst, u32 dst_offset = 0)
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

        static inline void copy_buffer(daxa::TaskInterface ti, daxa::TaskBuffer src, daxa::TaskBuffer dst, u32 dst_offset = 0)
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

        static inline void clear_buffer(daxa::TaskInterface ti, daxa::TaskBufferAttachmentInfo buffer, u32 value = 0)
        {
            ti.recorder.clear_buffer(
            {
                .buffer = buffer.ids[0],
                .offset = 0,
                .size = ti.device.buffer_info(buffer.ids[0]).value().size,
                .clear_value = value
            });
        }

        static inline void clear_buffer(daxa::TaskInterface ti, daxa::TaskBuffer buffer, u32 value = 0)
        {
            clear_buffer(ti, ti.get(buffer), value); // Update call
        }

        static inline void copy_image(daxa::TaskInterface ti, daxa::TaskImage src, daxa::TaskImage dst)
        {
            ti.recorder.copy_image_to_image({.src_image = ti.get(src).ids[0],
                                             .dst_image = ti.get(dst).ids[0]});
        }

        static inline DeviceAddress get_device_address(daxa::TaskInterface ti, daxa::TaskBuffer buffer, usize bufferIndex = 0)
        {
            return ti.device.buffer_device_address(ti.get(buffer).ids[bufferIndex]).value();
        }

        static inline daxa::Swapchain create_swapchain(daxa::Device & device, std::shared_ptr<GameWindow> window, std::string name)
        {
            auto native_handle = window->get_native_handle(); // Fix call
            auto platform = window->get_native_platform(); // Fix call

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

        static inline daxa::TaskImage create_swapchain_image(daxa::Swapchain & swapchain, std::string name)
        {
            return daxa::TaskImage{{.swapchain_image = true, .name = name}};
        }

        static inline daxa::PipelineManager create_pipeline_manager(daxa::Device & device, std::vector<std::filesystem::path> shaderDirectories, std::string name)
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
