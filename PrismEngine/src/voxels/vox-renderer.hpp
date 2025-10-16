#pragma once

//=============================================================================
// VoxelRenderer: Sparse 4x4x4 Contree Rendering System
//=============================================================================
// - 12-byte nodes (96 bits) for cache efficiency
// - CPU-side tree building with flat array layout
// - GPU DDA-based hierarchical traversal
// - 64-child branching factor (4x4x4) for shallow trees
//=============================================================================

#include <shaders/shared.inl>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <engine/data/Image.hpp>
#include <engine/apis/Math.hpp>
#include <engine/apis/Daxa.hpp>
#include <engine/objects/Renderer.hpp>
#include <engine/objects/Camera.hpp>
#include <chrono>
#include "contree.hpp"

using namespace daxa::types;

struct GBufferCPU
{
    daxa::ImageId color;
    daxa::ImageId normal;
    daxa::ImageId position;
    daxa::ImageId depth;
    daxa::ImageId ao;

    daxa::TaskImage task_color;
    daxa::TaskImage task_normal;
    daxa::TaskImage task_position;
    daxa::TaskImage task_depth;
    daxa::TaskImage task_ao;

    std::vector<daxa::ImageId> images;
    std::vector<daxa::TaskImage> task_images;

    inline void CreateImages(Renderer &renderer)
    {
        color = renderer.CreateRenderImage("color", &task_color, daxa::Format::R32G32B32A32_SFLOAT);
        normal = renderer.CreateRenderImage("normal", &task_normal, daxa::Format::R16G16B16A16_SFLOAT);
        position = renderer.CreateRenderImage("position", &task_position, daxa::Format::R32G32B32A32_SFLOAT);
        depth = renderer.CreateRenderImage("depth", &task_depth, daxa::Format::R32_SFLOAT, 1);
        ao = renderer.CreateRenderImage("ao", &task_ao, daxa::Format::R32_SFLOAT, 0.5);

        images = {color, normal, position, depth, ao};
        task_images = {task_color, task_normal, task_position, task_depth, task_ao};
    }

    inline void ResizeImages(Renderer &renderer)
    {
        DestroyImages(renderer);
        CreateImages(renderer);
    }

    inline void DestroyImages(Renderer &renderer)
    {
        for (auto image : images)
        {
            renderer.DestroyImage(image);
        }
    }

    inline void UseImages(daxa::TaskGraph &task_graph)
    {
        for (auto &task_image : task_images)
        {
            task_graph.use_persistent_image(task_image);
        }
    }

    inline GBuffer GetGPUBuffer(int frame)
    {
        return
        {
            .color = color.default_view(),
            .normal = normal.default_view(),
            .position = position.default_view(),
            .depth = depth.default_view(),
            .ao = ao.default_view(),
        };
    }

    inline void TaskAddAll(InlineTask &task, daxa::TaskImageAccess access)
    {
        for (auto &task_image : task_images)
        {
            task.AddAttachment(access, task_image);
        }
    }
};

class VoxelRenderer
{
        std::shared_ptr<Renderer> renderer;

        std::shared_ptr<daxa::ComputePipeline> render_gbuffer_compute;
        std::shared_ptr<daxa::ComputePipeline> render_composite_compute;

        daxa::BufferId materialsBuffer;
        daxa::TaskBuffer task_materialsBuffer;

        daxa::BufferId stateBuffer;
        daxa::TaskBuffer task_stateBuffer;

        daxa::BufferId contreeBuffer;
        daxa::TaskBuffer task_contreeBuffer;
        uint32_t contreeNodeCount = 0;

        std::unique_ptr<Image> blue_noise_images_data[64];
        daxa::ImageId blue_noise_images[64];
        daxa::TaskImage task_blue_noise_image;

        GBufferCPU gbufferCPU;
        daxa::BufferId gbufferGPU;
        daxa::TaskBuffer task_gbufferGPU;

        ComputePush pushData;

        // CPU-side timing (captured inside tasks)
        GPUTimings gpuTimings = {};
        std::chrono::high_resolution_clock::time_point frameStart;
        std::chrono::high_resolution_clock::time_point terrainStart, terrainEnd;
        std::chrono::high_resolution_clock::time_point gbufferStart, gbufferEnd;
        std::chrono::high_resolution_clock::time_point gtaoStart, gtaoEnd;
        std::chrono::high_resolution_clock::time_point aoApplyStart, aoApplyEnd;
        std::chrono::high_resolution_clock::time_point compositeStart, compositeEnd;

    public:
        Camera camera;
        RenderData stateData =
        {
            .sunDir = {1.0f, 0.2f, 0.1f},
            .frame = 0,
            .padding = {},
            .camera = {},
            .settings = {},
        };

        bool dirtyTerrain = true;
        bool resetLighting = true;

        VoxelRenderer(std::shared_ptr<Renderer> renderer)
            : renderer(renderer)
        {
            render_gbuffer_compute = renderer->AddComputePipeline<ComputePush>("voxel_raymarch", "render-gbuffer.slang");
            render_composite_compute = renderer->AddComputePipeline<ComputePush>("composite", "composite.slang");

            // Initialize buffers
            renderer->CreateBuffer<Materials>("Materials", materialsBuffer, task_materialsBuffer, daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE);
            renderer->CreateBuffer<RenderData>("RenderData", stateBuffer, task_stateBuffer, daxa::MemoryFlagBits::DEDICATED_MEMORY);
            renderer->CreateBuffer<GBuffer>("GBuffer", gbufferGPU, task_gbufferGPU, daxa::MemoryFlagBits::DEDICATED_MEMORY);

            gbufferCPU.CreateImages(*renderer);

            // load blue noise images
            for (int i = 0; i < 64; i++)
            {
                auto filename = std::to_string(i) + ".png";
                auto image = std::make_unique<Image>("resources/textures/noise/vec3/" + filename);
                blue_noise_images[i] = renderer->CreateImage(filename, *image);
                blue_noise_images_data[i] = std::move(image);
            }

            // create noise task image
            task_blue_noise_image = daxa::TaskImage(daxa::TaskImageInfo{.initial_images = {.images = blue_noise_images}, .name = "task_blue_noise_image"});

            float glowStrength = 2;

            // push default materials
            auto materials_ptr = renderer->MapBufferAs<Materials>(materialsBuffer);
            materials_ptr->data[0] = Material
            {
                .albedo = {1.0, 0, 1.0},
                .emission = {0.0f, 0.0f, 0.0f},
                .transparency = 0.0f,
                .roughness = 1,
            };
            materials_ptr->data[1] = Material // grass
            {
                .albedo = {0.17f, 0.33f, 0.23f},
                .emission = {0.0f, 0.0f, 0.0f},
                .transparency = 0.0f,
                .roughness = 0.5f,
            };
            materials_ptr->data[2] = Material // stone
            {
                .albedo = {0.5, 0.5, 0.5},
                .emission = {0.0f, 0.0f, 0.0f},
                .transparency = 0.0f,
                .roughness = 0.5f,
            };
            materials_ptr->data[3] = Material // dirt
            {
                .albedo = {0.14f, 0.09f, 0.02f},
                .emission = {0.0f, 0.0f, 0.0f},
                .transparency = 0.0f,
                .roughness = 0.5f,
            };
            materials_ptr->data[4] = Material // leaves
            {
                .albedo = {0.08f, 0.17f, 0.03f},
                .emission = {0.0f, 0.0f, 0.0f},
                .transparency = 0.0f,
                .roughness = 0.5f,
            };
            materials_ptr->data[5] = Material // wood
            {
                .albedo = {0.17f, 0.09f, 0.03f},
                .emission = {0.0f, 0.0f, 0.0f},
                .transparency = 0.0f,
                .roughness = 0.5f,
            };
            materials_ptr->data[6] = Material // purple glow
            {
                .albedo = {0.8, 0.3, 1},
                .emission = {0.8f * glowStrength, 0.3f * glowStrength, 1 * glowStrength},
                .transparency = 0.0f,
                .roughness = 0.5f,
            };
            materials_ptr->data[7] = Material // rblue glow
            {
                .albedo = {0.29f, 0.62f, 1},
                .emission = {0.29f * glowStrength, 0.62f * glowStrength, 1 * glowStrength},
                .transparency = 0.0f,
                .roughness = 0.5f,
            };
            materials_ptr->data[8] = Material
            {
                .albedo = {0.3f, 0.79f, 0.69f},
                .emission = {0.3f * glowStrength, 1.0f * glowStrength, 0.69f * glowStrength},
                .transparency = 0.0f,
                .roughness = 0.5f,
            };
            materials_ptr->data[9] = Material // water
            {
                .albedo = {0.3f, 0.79f, 0.69f},
                .emission = {0, 0, 0},
                .volumetricAbsorbtion = {0.35, 0.062, 0.018},
                .transparency = 0.5f,
                .reflectivity = 0.2f,
            };
            materials_ptr->data[10] = Material // mirror
            {
                .albedo = {0.9, 1, 0.9},
                .emission = {0, 0, 0},
                .transparency = 0.0f,
                .reflectivity = 0.9f,
            };
            materials_ptr->data[11] = Material // sand
            {
                .albedo = {0.76f, 0.70f, 0.50f},
                .emission = {0.0f, 0.0f, 0.0f},
                .transparency = 0.0f,
                .roughness = 0.8f,
            };

            // Build contree
            Contree::ContreeBuilder builder;
            builder.Build([](int x, int y, int z) -> uint16_t {
                // Create a hollow sphere at center for easy verification
                // Tree covers [0, 256)^3, so center at 128
                float centerX = 128.0f;
                float centerY = 128.0f;
                float centerZ = 128.0f;
                float dx = x - centerX;
                float dy = y - centerY;
                float dz = z - centerZ;
                float distSq = dx*dx + dy*dy + dz*dz;
                float outerRadiusSq = 60.0f * 60.0f;  // Radius 60
                float innerRadiusSq = 50.0f * 50.0f;  // Inner radius 50

                // Hollow sphere shell
                if (distSq < outerRadiusSq && distSq > innerRadiusSq) {
                    return 1; // Material 1 = grass (green)
                }
                return 0; // Empty
            });

            contreeNodeCount = builder.GetNodeCount();

            // Debug output
            std::printf("Contree built: %u nodes, %zu bytes\n",
                       contreeNodeCount, builder.GetSizeBytes());
            std::printf("Contree root PopMask: 0x%llx\n",
                       (unsigned long long)builder.GetNodes()[0].GetPopMask());
            std::printf("Camera will be at (128, 128, 400) looking at sphere at (128, 128, 128)\n");

            // Create buffer for contree data
            renderer->CreateBuffer(
                "ContreeBuffer",
                builder.GetSizeBytes(),
                contreeBuffer,
                task_contreeBuffer,
                daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE
            );

            // Copy contree data to GPU
            auto contreePtr = renderer->device.get_host_address_as<Contree::ContreeNode>(contreeBuffer).value();
            std::memcpy(contreePtr, builder.GetNodes().data(), builder.GetSizeBytes());

            frameStart = std::chrono::high_resolution_clock::now();
        }

        ~VoxelRenderer()
        {
            renderer->DestroyBuffer(materialsBuffer);
            renderer->DestroyBuffer(stateBuffer);
            renderer->DestroyBuffer(gbufferGPU);
            renderer->DestroyBuffer(contreeBuffer);

            for (int i = 0; i < 64; i++)
            {
                renderer->DestroyImage(blue_noise_images[i]);
            }

            gbufferCPU.DestroyImages(*renderer);
        }

        void Update()
        {
            // rotate sun around y axis
            // stateData.sunDir = daxa_transform_normal(stateData.sunDir, daxa_mat_from_axis_angle({0.0f, 1.0f, 0.0f}, 0.01f * stateData.dt));

            if (Input::WasKeyPressed(Key::Num1))
            {
                dirtyTerrain = true;
            }
        }

        void InitializeTasks(daxa::TaskGraph &task_graph, daxa::TaskImage *task_render_image, daxa::ImageId *render_image)
        {
            task_graph.use_persistent_image(*task_render_image);
            task_graph.use_persistent_buffer(task_materialsBuffer);
            task_graph.use_persistent_buffer(task_stateBuffer);
            task_graph.use_persistent_buffer(task_contreeBuffer);
            task_graph.use_persistent_image(task_blue_noise_image);
            task_graph.use_persistent_buffer(task_gbufferGPU);
            gbufferCPU.UseImages(task_graph);

            renderer->AddTask(InlineTask("setup_gbuffer")
                              .AddAllAttachments(daxa::TaskBufferAccess::COMPUTE_SHADER_WRITE, {task_stateBuffer, task_gbufferGPU})
                              .AddAllAttachments(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT, gbufferCPU.task_images)
                              .SetTask(
                                  [this](daxa::TaskInterface ti)
            {
                renderer->CopyToBuffer<GBuffer>(ti, gbufferCPU.GetGPUBuffer(stateData.frame), ti.get(task_gbufferGPU), 0);

                stateData.camera = camera.getCameraData();
                renderer->CopyToBuffer<RenderData>(ti, stateData, ti.get(task_stateBuffer), 0);
            }));

            // Terrain generation now happens on CPU during initialization
            // Old GPU terrain generation task removed


            renderer->AddTask(InlineTask("main_render_gbuffer")
                              .AddAllAttachments(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, {task_materialsBuffer, task_contreeBuffer})
                              .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE, task_stateBuffer)
                              .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_blue_noise_image)
                              .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_gbufferGPU)
                              .AddAllAttachments(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, gbufferCPU.task_images)
                              .SetTask([this, render_image](daxa::TaskInterface ti)
            {
                auto push = ComputePush
                {
                    .gbuffer = renderer->GetDeviceAddress(ti, task_gbufferGPU),
                    .materialsBuffer = renderer->GetDeviceAddress(ti, task_materialsBuffer),
                    .stateBuffer = renderer->GetDeviceAddress(ti, task_stateBuffer),
                    .contreeBuffer = renderer->GetDeviceAddress(ti, task_contreeBuffer),
                    .blueNoise = blue_noise_images[stateData.frame % 64].default_view(),
                    .screenSize = {renderer->surface_width, renderer->surface_height},
                    .contreeNodeCount = contreeNodeCount
                };

                ti.recorder.set_pipeline(*render_gbuffer_compute);

                // Pass 0: Trace rays and write GBuffer
                // Now using 8x8 workgroups (64 threads) for shared memory optimization
                push.passNum = 0;
                ti.recorder.push_constant(push);
                ti.recorder.dispatch({(renderer->surface_width + 7) / 8, (renderer->surface_height + 7) / 8});

                // Pass 1: Calculate GTAO from complete GBuffer
                push.passNum = 1;
                ti.recorder.push_constant(push);
                ti.recorder.dispatch({(renderer->surface_width + 7) / 8, (renderer->surface_height + 7) / 8});

                // Pass 2: Apply AO to lighting
                push.passNum = 2;
                ti.recorder.push_constant(push);
                ti.recorder.dispatch({(renderer->surface_width + 7) / 8, (renderer->surface_height + 7) / 8});
            }));

            renderer->AddTask(InlineTask("composite_render")
                              .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, *task_render_image)
                              .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_materialsBuffer)
                              .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE, task_stateBuffer)
                              .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_gbufferGPU)
                              .AddAllAttachments(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_images)
                              .SetTask(
                                  [this, render_image](daxa::TaskInterface ti)
            {

                auto push = ComputePush
                {
                    .gbuffer = renderer->GetDeviceAddress(ti, task_gbufferGPU),
                    .materialsBuffer = renderer->GetDeviceAddress(ti, task_materialsBuffer),
                    .stateBuffer = renderer->GetDeviceAddress(ti, task_stateBuffer),
                    .screen = render_image->default_view(),
                    .screenSize = {renderer->surface_width, renderer->surface_height}};

                ti.recorder.set_pipeline(*render_composite_compute);
                ti.recorder.push_constant(push);
                ti.recorder.dispatch({(renderer->surface_width + 7) / 8, (renderer->surface_height + 7) / 8});

            }));

            // create task to blit render image to swapchain
            renderer->AddTask(renderer->CreateSwapchainBlitTask(*task_render_image));
        }

        void Resize(u32 width, u32 height)
        {
            gbufferCPU.ResizeImages(*renderer);
            camera.setViewportSize(width, height);
        }
};
