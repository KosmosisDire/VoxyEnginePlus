#pragma once

#include <shaders/shared.inl>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <engine/data/Image.hpp>
#include <engine/apis/Math.hpp>
#include <engine/apis/Daxa.hpp>
#include <engine/objects/Renderer.hpp>
#include <engine/objects/Camera.hpp>

using namespace daxa::types;

struct GBufferCPU
{
    daxa::ImageId color;
    daxa::ImageId normal;
    daxa::ImageId position;
    daxa::ImageId voxelIDs;
    daxa::ImageId voxelFaceIDs;
    daxa::ImageId materialIDs;
    daxa::ImageId voxelUVs;
    daxa::ImageId brickUVs;
    daxa::ImageId motion;
    daxa::ImageId depth;
    daxa::ImageId ao;
    daxa::ImageId aoDenoised;
    daxa::ImageId indirectLight;
    daxa::ImageId directLight;

    daxa::ImageId history;
    daxa::ImageId currentFrame;

    daxa::TaskImage task_color;
    daxa::TaskImage task_normal;
    daxa::TaskImage task_position;
    daxa::TaskImage task_voxelIDs;
    daxa::TaskImage task_voxelFaceIDs;
    daxa::TaskImage task_materialIDs;
    daxa::TaskImage task_voxelUVs;
    daxa::TaskImage task_brickUVs;
    daxa::TaskImage task_motion;
    daxa::TaskImage task_depth;
    daxa::TaskImage task_ao;
    daxa::TaskImage task_aoDenoised;
    daxa::TaskImage task_indirectLight;
    daxa::TaskImage task_directLight;

    daxa::TaskImage task_history;
    daxa::TaskImage task_currentFrame;

    std::vector<daxa::ImageId> images;
    std::vector<daxa::TaskImage> task_images;

    inline void CreateImages(Renderer &renderer)
    {
        color = renderer.CreateRenderImage("color", &task_color, daxa::Format::R32G32B32A32_SFLOAT);
        normal = renderer.CreateRenderImage("normal", &task_normal, daxa::Format::R16G16B16A16_SFLOAT);
        position = renderer.CreateRenderImage("position", &task_position, daxa::Format::R32G32B32A32_SFLOAT);
        voxelUVs = renderer.CreateRenderImage("voxelUVs", &task_voxelUVs, daxa::Format::R16G16_SFLOAT);
        brickUVs = renderer.CreateRenderImage("brickUVs", &task_brickUVs, daxa::Format::R16G16_SFLOAT);
        motion = renderer.CreateRenderImage("motion", &task_motion, daxa::Format::R32G32_SFLOAT);
        depth = renderer.CreateRenderImage("depth", &task_depth, daxa::Format::R32_SFLOAT, 1);
        voxelIDs = renderer.CreateRenderImage("voxelIDs", &task_voxelIDs, daxa::Format::R32G32_UINT, 1.0);
        voxelFaceIDs = renderer.CreateRenderImage("voxelFaceIDs", &task_voxelFaceIDs, daxa::Format::R32_UINT, 1.0);
        materialIDs = renderer.CreateRenderImage("materialIDs", &task_materialIDs, daxa::Format::R32_UINT, 1.0);
        ao = renderer.CreateRenderImage("ao", &task_ao, daxa::Format::R32_SFLOAT, 0.5);
        aoDenoised = renderer.CreateRenderImage("aoDenoised", &task_aoDenoised, daxa::Format::R32_SFLOAT, 0.5);
        indirectLight = renderer.CreateRenderImage("indirectLight", &task_indirectLight, daxa::Format::R32G32B32A32_SFLOAT);
        directLight = renderer.CreateRenderImage("directLight", &task_directLight, daxa::Format::R32G32B32A32_SFLOAT);
        history = renderer.CreateRenderImage("history", &task_history, daxa::Format::R32G32B32A32_SFLOAT);
        currentFrame = renderer.CreateRenderImage("currentFrame", &task_currentFrame, daxa::Format::R32G32B32A32_SFLOAT);

        images = {color, normal, position, voxelUVs, brickUVs, motion, history, depth, voxelIDs, voxelFaceIDs, materialIDs, currentFrame, ao, aoDenoised, indirectLight, directLight};
        task_images = {task_color, task_normal, task_position, task_voxelUVs, task_brickUVs, task_motion, task_history, task_depth, task_voxelIDs, task_voxelFaceIDs, task_materialIDs, task_currentFrame, task_ao, task_aoDenoised, task_indirectLight, task_directLight};
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
            .voxelUVs = voxelUVs.default_view(),
            .brickUVs = brickUVs.default_view(),
            .motion = motion.default_view(),
            .depth = depth.default_view(),
            .voxelIDs = voxelIDs.default_view(),
            .voxelFaceIDs = voxelFaceIDs.default_view(),
            .materialIDs = materialIDs.default_view(),
            .ao = ao.default_view(),
            .aoDenoised = aoDenoised.default_view(),
            .indirectLight = indirectLight.default_view(),
            .directLight = directLight.default_view(),
            .history = frame % 2 == 0 ? history.default_view() : currentFrame.default_view(),
            .currentFrame = frame % 2 == 0 ? currentFrame.default_view() : history.default_view(),
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

        std::shared_ptr<daxa::ComputePipeline> terrain_compute;
        std::shared_ptr<daxa::ComputePipeline> render_gbuffer_compute;
        std::shared_ptr<daxa::ComputePipeline> denoise_compute;
        std::shared_ptr<daxa::ComputePipeline> render_composite_compute;

        daxa::BufferId chunksBuffer;
        daxa::TaskBuffer task_chunksBuffer;

        daxa::BufferId bricksBuffer;
        daxa::TaskBuffer task_bricksBuffer;

        daxa::BufferId brickPtrBuffer;
        daxa::TaskBuffer task_brickPtrBuffer;

        daxa::BufferId materialsBuffer;
        daxa::TaskBuffer task_materialsBuffer;

        daxa::BufferId materialPtrBuffer;
        daxa::TaskBuffer task_materialPtrBuffer;

        daxa::BufferId stateBuffer;
        daxa::TaskBuffer task_stateBuffer;

        std::unique_ptr<Image> blue_noise_images_data[64];
        daxa::ImageId blue_noise_images[64];
        daxa::TaskImage task_blue_noise_image;

        GBufferCPU gbufferCPU;
        daxa::BufferId gbufferGPU;
        daxa::TaskBuffer task_gbufferGPU;

        ComputePush pushData;

    public:
        Camera camera;
        RenderData stateData =
        {
            .sunDir = {1.0f, 0.2f, 0.1f},
            .frame = 0,
            .padding = {},
            .camera = {},
            .lastCamera = {},
            .settings = {},
        };

        bool dirtyTerrain = true;
        bool resetLighting = true;

        VoxelRenderer(std::shared_ptr<Renderer> renderer)
            : renderer(renderer)
        {
            terrain_compute = renderer->AddComputePipeline<ComputePush>("terrain_gen", "terrain-gen.slang");
            render_gbuffer_compute = renderer->AddComputePipeline<ComputePush>("voxel_raymarch", "render-gbuffer.slang");
            denoise_compute = renderer->AddComputePipeline<ComputePush>("denoise", "denoise.slang");
            render_composite_compute = renderer->AddComputePipeline<ComputePush>("composite", "composite.slang");

            // Initialize occupancy buffer
            renderer->CreateBuffer<Chunks>("Chunks", chunksBuffer, task_chunksBuffer);
            renderer->CreateBuffer<Bricks>("Bricks", bricksBuffer, task_bricksBuffer);
            renderer->CreateBuffer<BrickPointers>("BrickPointers", brickPtrBuffer, task_brickPtrBuffer);
            renderer->CreateBuffer<Materials>("Materials", materialsBuffer, task_materialsBuffer, daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE);
            renderer->CreateBuffer<MaterialPointers>("MaterialPointers", materialPtrBuffer, task_materialPtrBuffer);
            renderer->CreateBuffer<RenderData>("RenderData", stateBuffer, task_stateBuffer);
            renderer->CreateBuffer<GBuffer>("GBuffer", gbufferGPU, task_gbufferGPU);

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
        }

        ~VoxelRenderer()
        {
            renderer->DestroyBuffer(chunksBuffer);
            renderer->DestroyBuffer(bricksBuffer);
            renderer->DestroyBuffer(materialsBuffer);
            renderer->DestroyBuffer(materialPtrBuffer);
            renderer->DestroyBuffer(stateBuffer);
            renderer->DestroyBuffer(gbufferGPU);

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
            task_graph.use_persistent_buffer(task_chunksBuffer);
            task_graph.use_persistent_buffer(task_bricksBuffer);
            task_graph.use_persistent_buffer(task_brickPtrBuffer);
            task_graph.use_persistent_buffer(task_materialsBuffer);
            task_graph.use_persistent_buffer(task_materialPtrBuffer);
            task_graph.use_persistent_buffer(task_stateBuffer);
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

                stateData.lastCamera = stateData.camera;
                stateData.camera = camera.getCameraData();
                renderer->CopyToBuffer<RenderData>(ti, stateData, ti.get(task_stateBuffer), 0);
            }));

            renderer->AddTask(
                InlineTask("terrain_gen")
                .AddAllAttachments(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, {task_chunksBuffer, task_bricksBuffer, task_brickPtrBuffer, task_materialPtrBuffer})
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_stateBuffer)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
            {
                if (!dirtyTerrain)
                    return;

                auto push = ComputePush
                {
                    .chunksBuffer = renderer->GetDeviceAddress(ti, task_chunksBuffer, 0),
                    .bricksBuffer = renderer->GetDeviceAddress(ti, task_bricksBuffer, 0),
                    .brickPtrBuffer = renderer->GetDeviceAddress(ti, task_brickPtrBuffer, 0),
                    .materialPtrBuffer = renderer->GetDeviceAddress(ti, task_materialPtrBuffer, 0),
                    .stateBuffer = renderer->GetDeviceAddress(ti, task_stateBuffer, 0)
                };
                ti.recorder.set_pipeline(*terrain_compute);

                push.passNum = 1;
                ti.recorder.push_constant(push);
                ti.recorder.dispatch({(GRID_SIZE * CHUNK_SIZE + 3) / 4, (GRID_SIZE * CHUNK_SIZE + 3) / 4, (GRID_SIZE * CHUNK_SIZE + 3) / 4});

                push.passNum = 2;
                ti.recorder.push_constant(push);
                ti.recorder.dispatch({(GRID_SIZE * CHUNK_SIZE + 3) / 4, (GRID_SIZE * CHUNK_SIZE + 3) / 4, (GRID_SIZE * CHUNK_SIZE + 3) / 4});

                dirtyTerrain = false;
            }));


            renderer->AddTask(InlineTask("main_render_gbuffer")
                              .AddAllAttachments(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, {task_chunksBuffer, task_bricksBuffer, task_brickPtrBuffer, task_materialPtrBuffer, task_materialsBuffer})
                              .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE, task_stateBuffer)
                              .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_blue_noise_image)
                              .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_gbufferGPU)
                              .AddAllAttachments(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, gbufferCPU.task_images)
                              .SetTask([this, render_image](daxa::TaskInterface ti)
            {
                auto push = ComputePush
                {
                    .gbuffer = renderer->GetDeviceAddress(ti, task_gbufferGPU),
                    .chunksBuffer = renderer->GetDeviceAddress(ti, task_chunksBuffer),
                    .bricksBuffer = renderer->GetDeviceAddress(ti, task_bricksBuffer),
                    .brickPtrBuffer = renderer->GetDeviceAddress(ti, task_brickPtrBuffer),
                    .materialsBuffer = renderer->GetDeviceAddress(ti, task_materialsBuffer),
                    .materialPtrBuffer = renderer->GetDeviceAddress(ti, task_materialPtrBuffer),
                    .stateBuffer = renderer->GetDeviceAddress(ti, task_stateBuffer),
                    .blueNoise = blue_noise_images[stateData.frame % 64].default_view(),
                    .blueNoiseStatic = blue_noise_images[0].default_view(),
                    .screenSize = {renderer->surface_width, renderer->surface_height}
                };

                push.passNum = 0;
                ti.recorder.set_pipeline(*render_gbuffer_compute);
                ti.recorder.push_constant(push);
                ti.recorder.dispatch({(renderer->surface_width + 31) / 32, (renderer->surface_height + 31) / 32});

                // denoise AO
                ti.recorder.set_pipeline(*denoise_compute);

                for (int i = 1; i <= 5; i++)
                {
                    push.passNum = i;
                    ti.recorder.push_constant(push);
                    ti.recorder.dispatch({(renderer->surface_width / 2 + 15) / 16, (renderer->surface_height / 2 + 15) / 16});
                }

                // combine gbuffer
                push.passNum = 1;
                ti.recorder.set_pipeline(*render_gbuffer_compute);
                ti.recorder.push_constant(push);
                ti.recorder.dispatch({(renderer->surface_width + 31) / 32, (renderer->surface_height + 31) / 32});

                // TAA
                push.passNum = 2;
                ti.recorder.push_constant(push);
                ti.recorder.dispatch({(renderer->surface_width + 31) / 32, (renderer->surface_height + 31) / 32});
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
