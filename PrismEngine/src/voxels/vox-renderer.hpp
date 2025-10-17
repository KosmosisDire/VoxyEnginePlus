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

        daxa::BufferId brickBuffer;
        daxa::TaskBuffer task_brickBuffer;
        uint32_t brickCount = 0;

        daxa::BufferId brickPaletteBuffer;
        daxa::TaskBuffer task_brickPaletteBuffer;

        daxa::BufferId brickIndexBuffer;
        daxa::TaskBuffer task_brickIndexBuffer;

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

            // Build contree with SIMPLE test geometry (sphere)
            Contree::ContreeBuilder builder;

            // METHOD 1: Direct function-based generation (current approach)
            // builder.Build([](int x, int y, int z) -> uint16_t {
            //     // Simple sphere at center (128, 128, 128) with radius 50
            //     float dx = x - 128.0f;
            //     float dy = y - 128.0f;
            //     float dz = z - 128.0f;
            //     float distSq = dx*dx + dy*dy + dz*dz;
            //     float radius = 50.0f;

            //     if (distSq < radius * radius) {
            //         // Different materials based on distance from center
            //         float dist = std::sqrt(distSq);
            //         if (dist < radius * 0.3f) {
            //             return 6; // Purple glow (core)
            //         } else if (dist < radius * 0.6f) {
            //             return 2; // Stone (middle layer)
            //         } else {
            //             return 1; // Grass (outer layer)
            //         }
            //     }

            //     return 0; // Empty
            // });

            // METHOD 2: Grid-based generation (for complex terrain)
            // Example: Generate a 256x256x256 voxel grid with advanced terrain
            
            // Simple 3D noise function
            auto hash = [](int x, int y, int z) -> float {
                int n = x + y * 57 + z * 113;
                n = (n << 13) ^ n;
                return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
            };

            auto noise3D = [&hash](float x, float y, float z) -> float {
                int ix = (int)std::floor(x);
                int iy = (int)std::floor(y);
                int iz = (int)std::floor(z);
                float fx = x - ix;
                float fy = y - iy;
                float fz = z - iz;

                fx = fx * fx * (3.0f - 2.0f * fx);
                fy = fy * fy * (3.0f - 2.0f * fy);
                fz = fz * fz * (3.0f - 2.0f * fz);

                float c000 = hash(ix, iy, iz);
                float c100 = hash(ix+1, iy, iz);
                float c010 = hash(ix, iy+1, iz);
                float c110 = hash(ix+1, iy+1, iz);
                float c001 = hash(ix, iy, iz+1);
                float c101 = hash(ix+1, iy, iz+1);
                float c011 = hash(ix, iy+1, iz+1);
                float c111 = hash(ix+1, iy+1, iz+1);

                float c00 = c000 * (1-fx) + c100 * fx;
                float c01 = c001 * (1-fx) + c101 * fx;
                float c10 = c010 * (1-fx) + c110 * fx;
                float c11 = c011 * (1-fx) + c111 * fx;

                float c0 = c00 * (1-fy) + c10 * fy;
                float c1 = c01 * (1-fy) + c11 * fy;

                return c0 * (1-fz) + c1 * fz;
            };

            constexpr int GRID_SIZE = 256;
            std::vector<uint16_t> voxelGrid(GRID_SIZE * GRID_SIZE * GRID_SIZE, 0);

            // Generate complex terrain into the grid
            for (int z = 0; z < GRID_SIZE; z++) {
                for (int y = 0; y < GRID_SIZE; y++) {
                    for (int x = 0; x < GRID_SIZE; x++) {
                        int idx = x + y * GRID_SIZE + z * GRID_SIZE * GRID_SIZE;

                        // Example: Multi-octave noise terrain
                        float height = 100.0f;
                        height += noise3D(x * 0.01f, z * 0.01f, 0.0f) * 50.0f;  // Large features
                        height += noise3D(x * 0.05f, z * 0.05f, 0.0f) * 10.0f;  // Medium details
                        height += noise3D(x * 0.1f, z * 0.1f, 0.0f) * 3.0f;     // Fine details

                        if (y < height) {
                            // Layer materials by depth
                            if (y > height - 3) {
                                voxelGrid[idx] = 1; // Grass
                            } else if (y > height - 10) {
                                voxelGrid[idx] = 3; // Dirt
                            } else {
                                voxelGrid[idx] = 2; // Stone
                            }
                        } else {
                            voxelGrid[idx] = 0; // Air
                        }
                    }
                }
            }

            // Build contree from the pre-generated grid
            builder.BuildFromGrid(voxelGrid.data(), GRID_SIZE, GRID_SIZE, GRID_SIZE);
            

            contreeNodeCount = builder.GetNodeCount();
            brickCount = builder.GetBrickCount();

            // Debug output
            std::printf("=== Simple Sphere Generated ===\n");
            std::printf("Contree nodes: %u (%.2f KB)\n",
                       contreeNodeCount, builder.GetSizeBytes() / 1024.0f);
            std::printf("Bricks: %u (%.2f KB)\n",
                       brickCount, builder.GetBricksSizeBytes() / 1024.0f);
            std::printf("Palette data: %.2f KB\n", builder.GetBrickPaletteDataSizeBytes() / 1024.0f);
            std::printf("Index data: %.2f KB\n", builder.GetBrickIndexDataSizeBytes() / 1024.0f);
            std::printf("Total memory: %.2f KB\n",
                       (builder.GetSizeBytes() + builder.GetBricksSizeBytes() +
                        builder.GetBrickPaletteDataSizeBytes() + builder.GetBrickIndexDataSizeBytes()) / 1024.0f);

            // Create buffers
            renderer->CreateBuffer(
                "ContreeBuffer",
                builder.GetSizeBytes(),
                contreeBuffer,
                task_contreeBuffer,
                daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE
            );

            renderer->CreateBuffer(
                "BrickBuffer",
                builder.GetBricksSizeBytes(),
                brickBuffer,
                task_brickBuffer,
                daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE
            );

            // Pack palette data: uint16s -> uint32s (2 per uint32)
            size_t paletteU16Count = builder.GetBrickPaletteData().size();
            size_t paletteU32Count = (paletteU16Count + 1) / 2; // Round up
            size_t paletteBufferSize = std::max(paletteU32Count * sizeof(uint32_t), size_t(4)); // Min 4 bytes

            // Pack index data: uint8s -> uint32s (4 per uint32)
            size_t indexU8Count = builder.GetBrickIndexData().size();
            size_t indexU32Count = (indexU8Count + 3) / 4; // Round up
            size_t indexBufferSize = std::max(indexU32Count * sizeof(uint32_t), size_t(4)); // Min 4 bytes

            renderer->CreateBuffer(
                "BrickPaletteBuffer",
                paletteBufferSize,
                brickPaletteBuffer,
                task_brickPaletteBuffer,
                daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE
            );

            renderer->CreateBuffer(
                "BrickIndexBuffer",
                indexBufferSize,
                brickIndexBuffer,
                task_brickIndexBuffer,
                daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE
            );

            // Copy data to GPU
            auto contreePtr = renderer->device.get_host_address_as<Contree::ContreeNode>(contreeBuffer).value();
            std::memcpy(contreePtr, builder.GetNodes().data(), builder.GetSizeBytes());

            auto brickPtr = renderer->device.get_host_address_as<Contree::BrickHeader>(brickBuffer).value();
            std::memcpy(brickPtr, builder.GetBricks().data(), builder.GetBricksSizeBytes());

            // Pack palette: uint16s into uint32s
            auto palettePtr = renderer->device.get_host_address_as<uint32_t>(brickPaletteBuffer).value();
            const uint16_t* paletteData = builder.GetBrickPaletteData().data();
            for (size_t i = 0; i < paletteU32Count; i++) {
                uint32_t packed = 0;
                if (i * 2 < paletteU16Count) {
                    packed |= paletteData[i * 2];
                }
                if (i * 2 + 1 < paletteU16Count) {
                    packed |= (uint32_t(paletteData[i * 2 + 1]) << 16);
                }
                palettePtr[i] = packed;
            }

            // Pack index: uint8s into uint32s
            auto indexPtr = renderer->device.get_host_address_as<uint32_t>(brickIndexBuffer).value();
            const uint8_t* indexData = builder.GetBrickIndexData().data();
            for (size_t i = 0; i < indexU32Count; i++) {
                uint32_t packed = 0;
                for (int j = 0; j < 4 && (i * 4 + j) < indexU8Count; j++) {
                    packed |= (uint32_t(indexData[i * 4 + j]) << (j * 8));
                }
                indexPtr[i] = packed;
            }

            frameStart = std::chrono::high_resolution_clock::now();
        }

        ~VoxelRenderer()
        {
            renderer->DestroyBuffer(materialsBuffer);
            renderer->DestroyBuffer(stateBuffer);
            renderer->DestroyBuffer(gbufferGPU);
            renderer->DestroyBuffer(contreeBuffer);
            renderer->DestroyBuffer(brickBuffer);
            renderer->DestroyBuffer(brickPaletteBuffer);
            renderer->DestroyBuffer(brickIndexBuffer);

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
            task_graph.use_persistent_buffer(task_brickBuffer);
            task_graph.use_persistent_buffer(task_brickPaletteBuffer);
            task_graph.use_persistent_buffer(task_brickIndexBuffer);
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
                              .AddAllAttachments(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, {task_materialsBuffer, task_contreeBuffer, task_brickBuffer, task_brickPaletteBuffer, task_brickIndexBuffer})
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
                    .brickBuffer = renderer->GetDeviceAddress(ti, task_brickBuffer),
                    .brickPaletteData = renderer->GetDeviceAddress(ti, task_brickPaletteBuffer),
                    .brickIndexData = renderer->GetDeviceAddress(ti, task_brickIndexBuffer),
                    .blueNoise = blue_noise_images[stateData.frame % 64].default_view(),
                    .screenSize = {renderer->surface_width, renderer->surface_height},
                    .contreeNodeCount = contreeNodeCount,
                    .brickCount = brickCount
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
