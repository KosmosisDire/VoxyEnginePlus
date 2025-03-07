#pragma once

#include "shaders/shared.inl"
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <math.hpp>
#include <math/daxa_math.hpp>
#include <renderer.hpp>
#include <image.hpp>

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

    daxa::ImageId indirect;
    daxa::ImageId indirectLast;
    daxa::ImageId indirectDenoised;
    daxa::ImageId indirectPerVoxelPass1;
    daxa::ImageId indirectPerVoxelPass2;
    daxa::ImageId motion;
    daxa::ImageId depth;
    daxa::ImageId depthHalfRes;
    daxa::ImageId ssao;
    daxa::ImageId shadow;

    daxa::TaskImage task_color;
    daxa::TaskImage task_normal;
    daxa::TaskImage task_position;
    daxa::TaskImage task_voxelIDs;
    daxa::TaskImage task_voxelFaceIDs;
    daxa::TaskImage task_materialIDs;
    daxa::TaskImage task_voxelUVs;
    daxa::TaskImage task_brickUVs;

    daxa::TaskImage task_indirect;
    daxa::TaskImage task_indirectLast;
    daxa::TaskImage task_indirectDenoised;
    daxa::TaskImage task_indirectPerVoxelPass1;
    daxa::TaskImage task_indirectPerVoxelPass2;
    daxa::TaskImage task_motion;
    daxa::TaskImage task_depth;
    daxa::TaskImage task_depthHalfRes;

    daxa::TaskImage task_ssao;
    daxa::TaskImage task_shadow;

    std::vector<daxa::ImageId> images;
    std::vector<daxa::TaskImage> task_images;

    inline void CreateImages(Renderer &renderer)
    {
        color = renderer.CreateRenderImage("color", &task_color, daxa::Format::R32G32B32A32_SFLOAT);
        normal = renderer.CreateRenderImage("normal", &task_normal, daxa::Format::R16G16B16A16_SFLOAT);
        position = renderer.CreateRenderImage("position", &task_position, daxa::Format::R32G32B32A32_SFLOAT);
        voxelUVs = renderer.CreateRenderImage("voxelUVs", &task_voxelUVs, daxa::Format::R16G16_SFLOAT);
        brickUVs = renderer.CreateRenderImage("brickUVs", &task_brickUVs, daxa::Format::R16G16_SFLOAT);
        indirect = renderer.CreateRenderImage("indirect", &task_indirect, daxa::Format::R32G32B32A32_SFLOAT, LIGHTING_UPSAMPLE);
        indirectLast = renderer.CreateRenderImage("indirectLast", &task_indirectLast, daxa::Format::R32G32B32A32_SFLOAT, LIGHTING_UPSAMPLE);
        indirectDenoised = renderer.CreateRenderImage("indirectDenoised", &task_indirectDenoised, daxa::Format::R32G32B32A32_SFLOAT, LIGHTING_UPSAMPLE);
        indirectPerVoxelPass1 = renderer.CreateRenderImage("indirectPerVoxelPass1", &task_indirectPerVoxelPass1, daxa::Format::R32G32B32A32_SFLOAT, LIGHTING_UPSAMPLE);
        indirectPerVoxelPass2 = renderer.CreateRenderImage("indirectPerVoxelPass2", &task_indirectPerVoxelPass2, daxa::Format::R32G32B32A32_SFLOAT, LIGHTING_UPSAMPLE);
        motion = renderer.CreateRenderImage("motion", &task_motion, daxa::Format::R32G32_SFLOAT);
        depth = renderer.CreateRenderImage("depth", &task_depth, daxa::Format::R32_SFLOAT, 1);
        depthHalfRes = renderer.CreateRenderImage("depthHalfRes", &task_depthHalfRes, daxa::Format::R32_SFLOAT, 0.5);
        voxelIDs = renderer.CreateRenderImage("voxelIDs", &task_voxelIDs, daxa::Format::R32G32_UINT, 1.0);
        voxelFaceIDs = renderer.CreateRenderImage("voxelFaceIDs", &task_voxelFaceIDs, daxa::Format::R32_UINT, 1.0);
        materialIDs = renderer.CreateRenderImage("materialIDs", &task_materialIDs, daxa::Format::R32_UINT, 1.0);
        ssao = renderer.CreateRenderImage("ssao", &task_ssao, daxa::Format::R32_SFLOAT, 0.5);
        shadow = renderer.CreateRenderImage("shadow", &task_shadow, daxa::Format::R32_SFLOAT, 1.0);

        images = {color, normal, position, voxelUVs, brickUVs, indirect, indirectLast, indirectDenoised, indirectPerVoxelPass1, indirectPerVoxelPass2, motion, depth, depthHalfRes, voxelIDs, materialIDs, voxelFaceIDs, ssao, shadow};
        task_images = {task_color, task_normal, task_position, task_voxelUVs, task_brickUVs, task_indirect, task_indirectLast, task_indirectDenoised, task_indirectPerVoxelPass1, task_indirectPerVoxelPass2, task_motion, task_depth, task_depthHalfRes, task_voxelIDs, task_voxelFaceIDs, task_materialIDs, task_ssao, task_shadow};
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
        return {
            .color = color.default_view(),
            .normal = normal.default_view(),
            .position = position.default_view(),
            .voxelUVs = voxelUVs.default_view(),
            .brickUVs = brickUVs.default_view(),
            .indirect = frame % 2 == 0 ? indirect.default_view() : indirectLast.default_view(),
            .indirectLast = frame % 2 == 0 ? indirectLast.default_view() : indirect.default_view(),
            .indirectDenoised = indirectDenoised.default_view(),
            .indirectPerVoxelPass1 = indirectPerVoxelPass1.default_view(),
            .indirectPerVoxelPass2 = indirectPerVoxelPass2.default_view(),
            .motion = motion.default_view(),
            .depth = depth.default_view(),
            .depthHalfRes = depthHalfRes.default_view(),
            .voxelIDs = voxelIDs.default_view(),
            .voxelFaceIDs = voxelFaceIDs.default_view(),
            .materialIDs = materialIDs.default_view(),
            .ssao = ssao.default_view(),
            .shadow = shadow.default_view(),
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
    std::shared_ptr<daxa::ComputePipeline> depth_prepass_compute;
    std::shared_ptr<daxa::ComputePipeline> render_gbuffer_compute;
    std::shared_ptr<daxa::ComputePipeline> denoise_gi_compute;
    std::shared_ptr<daxa::ComputePipeline> combine_gi_compute;
    std::shared_ptr<daxa::ComputePipeline> ssao_compute;
    std::shared_ptr<daxa::ComputePipeline> render_composite_compute;

    daxa::BufferId chunk_occupancy_buffer;
    daxa::TaskBuffer task_chunk_occupancy_buffer;

    daxa::BufferId brick_occupancy_buffer;
    daxa::TaskBuffer task_brick_occupancy_buffer;

    daxa::BufferId materials_buffer;
    daxa::TaskBuffer task_materials_buffer;

    daxa::BufferId voxel_materials_buffer;
    daxa::TaskBuffer task_voxel_materials_buffer;

    daxa::BufferId state_buffer;
    daxa::TaskBuffer task_state_buffer;

    daxa::BufferId voxel_hashmap_buffer;
    daxa::TaskBuffer task_voxel_hashmap_buffer;

    std::unique_ptr<Image> blue_noise_images_data[64];
    daxa::ImageId blue_noise_images[64];
    daxa::TaskImage task_blue_noise_image;

    GBufferCPU gbufferCPU;
    daxa::BufferId gbufferGPU;
    daxa::TaskBuffer task_gbufferGPU;

    ComputePush pushData;


  public:
    VoxCamera camera;
    RenderData stateData = {
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
        depth_prepass_compute = renderer->AddComputePipeline<ComputePush>("depth_prepass", "depth-prepass.slang");
        render_gbuffer_compute = renderer->AddComputePipeline<ComputePush>("voxel_raymarch", "render-gbuffer.slang");
        ssao_compute = renderer->AddComputePipeline<ComputePush>("ssao", "ssao.slang");
        denoise_gi_compute = renderer->AddComputePipeline<DenoisePush>("denoise_gi", "denoise-gi.slang");
        combine_gi_compute = renderer->AddComputePipeline<ComputePush>("combine_gi", "combine-gi.slang");
        render_composite_compute = renderer->AddComputePipeline<ComputePush>("composite", "composite.slang");

        // Initialize occupancy buffer
        renderer->CreateBuffer<ChunkOccupancy>("chunk_occupancy", chunk_occupancy_buffer, task_chunk_occupancy_buffer);
        renderer->CreateBuffer<BrickOccupancy>("brick_occupancy", brick_occupancy_buffer, task_brick_occupancy_buffer);
        renderer->CreateBuffer<Materials>("materials buffer", materials_buffer, task_materials_buffer, daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE);
        renderer->CreateBuffer<VoxelMaterials>("voxel materials buffer", voxel_materials_buffer, task_voxel_materials_buffer);
        renderer->CreateBuffer<RenderData>("state buffer", state_buffer, task_state_buffer);
        renderer->CreateBuffer<VoxelHashmap>("voxel hashmap buffer", voxel_hashmap_buffer, task_voxel_hashmap_buffer);
        renderer->CreateBuffer<GBuffer>("gbufferGPU", gbufferGPU, task_gbufferGPU);

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
        auto materials_ptr = renderer->MapBufferAs<Materials>(materials_buffer);
        materials_ptr->materials[0] = Material
        {
            .albedo = {1.0, 0, 1.0},
            .emission = {0.0f, 0.0f, 0.0f},
            .transparency = 0.0f,
            .roughness = 1,
        };
        materials_ptr->materials[1] = Material // grass
        {
            .albedo = {0.17f, 0.33f, 0.23f},
            .emission = {0.0f, 0.0f, 0.0f},
            .transparency = 0.0f,
            .roughness = 0.5f,
        };
        materials_ptr->materials[2] = Material // stone
        {
            .albedo = {0.5, 0.5, 0.5},
            .emission = {0.0f, 0.0f, 0.0f},
            .transparency = 0.0f,
            .roughness = 0.5f,
        };
        materials_ptr->materials[3] = Material // dirt
        {
            .albedo = {0.14f, 0.09f, 0.02f},
            .emission = {0.0f, 0.0f, 0.0f},
            .transparency = 0.0f,
            .roughness = 0.5f,
        };
        materials_ptr->materials[4] = Material // leaves
        {
            .albedo = {0.08f, 0.17f, 0.03f},
            .emission = {0.0f, 0.0f, 0.0f},
            .transparency = 0.0f,
            .roughness = 0.5f,
        };
        materials_ptr->materials[5] = Material // wood
        {
            .albedo = {0.17f, 0.09f, 0.03f},
            .emission = {0.0f, 0.0f, 0.0f},
            .transparency = 0.0f,
            .roughness = 0.5f,
        };
        materials_ptr->materials[6] = Material // purple glow
        {
            .albedo = {0.8, 0.3, 1},
            .emission = {0.8f * glowStrength, 0.3f * glowStrength, 1 * glowStrength},
            .transparency = 0.0f,
            .roughness = 0.5f,
        };
        materials_ptr->materials[7] = Material // rblue glow
        {
            .albedo = {0.29f, 0.62f, 1},
            .emission = {0.29f * glowStrength, 0.62f * glowStrength, 1 * glowStrength},
            .transparency = 0.0f,
            .roughness = 0.5f,
        };
        materials_ptr->materials[8] = Material
        {
            .albedo = {0.3f, 0.79f, 0.69f},
            .emission = {0.3f * glowStrength, 1.0f * glowStrength, 0.69f * glowStrength},
            .transparency = 0.0f,
            .roughness = 0.5f,
        };
        materials_ptr->materials[9] = Material // water
        {
            .albedo = {0.3f, 0.79f, 0.69f},
            .emission = {0,0,0},
            .volumetricAbsorbtion = {0.35, 0.062, 0.018},
            .transparency = 0.5f,
            .reflectivity = 0.2f,
        };
        materials_ptr->materials[10] = Material // mirror
        {
            .albedo = {0.9,1,0.9},
            .emission = {0,0,0},
            .transparency = 0.0f,
            .reflectivity = 0.9f,
        };



    }

    ~VoxelRenderer()
    {
        renderer->DestroyBuffer(chunk_occupancy_buffer);
        renderer->DestroyBuffer(brick_occupancy_buffer);
        renderer->DestroyBuffer(materials_buffer);
        renderer->DestroyBuffer(voxel_materials_buffer);
        renderer->DestroyBuffer(state_buffer);
        renderer->DestroyBuffer(voxel_hashmap_buffer);
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

        if (Input::WasKeyPressed(Key::Num2))
        {
            resetLighting = true;
        }
    }

    void InitializeTasks(daxa::TaskGraph &task_graph, daxa::TaskImage *task_render_image, daxa::ImageId *render_image)
    {
        task_graph.use_persistent_image(*task_render_image);
        task_graph.use_persistent_buffer(task_chunk_occupancy_buffer);
        task_graph.use_persistent_buffer(task_brick_occupancy_buffer);
        task_graph.use_persistent_buffer(task_materials_buffer);
        task_graph.use_persistent_buffer(task_voxel_materials_buffer);
        task_graph.use_persistent_buffer(task_voxel_hashmap_buffer);
        task_graph.use_persistent_buffer(task_state_buffer);
        task_graph.use_persistent_image(task_blue_noise_image);
        task_graph.use_persistent_buffer(task_gbufferGPU); 
        gbufferCPU.UseImages(task_graph);

        renderer->AddTask(InlineTask("setup_gbuffer")
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_WRITE, task_state_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_WRITE, task_gbufferGPU)
                .AddAllAttachments(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT, gbufferCPU.task_images)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        renderer->CopyToBuffer<GBuffer>(ti, gbufferCPU.GetGPUBuffer(stateData.frame), ti.get(task_gbufferGPU), 0);
                        
                        stateData.lastCamera = stateData.camera;
                        stateData.camera = camera.getCameraData();
                        renderer->CopyToBuffer<RenderData>(ti, stateData, ti.get(task_state_buffer), 0);

                        if (resetLighting)
                        {
                            renderer->ClearBuffer(ti, task_voxel_hashmap_buffer, 0);
                            resetLighting = false;
                        }
                    }));

        renderer->AddTask(
            InlineTask("terrain_gen")
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_chunk_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_brick_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_voxel_materials_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
                    {
                        if (!dirtyTerrain) return;
                            
                        auto push = ComputePush
                        {
                            .chunk_occupancy_ptr = renderer->GetDeviceAddress(ti, task_chunk_occupancy_buffer, 0),
                            .brick_occupancy_ptr = renderer->GetDeviceAddress(ti, task_brick_occupancy_buffer, 0),
                            .voxel_materials_ptr = renderer->GetDeviceAddress(ti, task_voxel_materials_buffer, 0),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer, 0)
                        };
                        ti.recorder.set_pipeline(*terrain_compute);

                        push.pass = 1;
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(GRID_SIZE * CHUNK_SIZE + 3) / 4, (GRID_SIZE * CHUNK_SIZE + 3) / 4, (GRID_SIZE * CHUNK_SIZE + 3) / 4});
                        
                        push.pass = 2;
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(GRID_SIZE * CHUNK_SIZE + 3) / 4, (GRID_SIZE * CHUNK_SIZE + 3) / 4, (GRID_SIZE * CHUNK_SIZE + 3) / 4});

                        dirtyTerrain = false;
                    }));

        // depth prepass
        renderer->AddTask(
            InlineTask("depth_prepass")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, gbufferCPU.task_depthHalfRes)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_chunk_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_brick_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_gbufferGPU)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush
                        {
                            .gbuffer = renderer->GetDeviceAddress(ti, task_gbufferGPU),
                            .chunk_occupancy_ptr = renderer->GetDeviceAddress(ti, task_chunk_occupancy_buffer),
                            .brick_occupancy_ptr = renderer->GetDeviceAddress(ti, task_brick_occupancy_buffer),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}
                        };

                        ti.recorder.set_pipeline(*depth_prepass_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({((renderer->surface_width / 2) + 7) / 8, ((renderer->surface_height / 2) + 7) / 8});
                    }));

        
        renderer->AddTask(InlineTask("main_render_gbuffer") 
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_chunk_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_brick_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_voxel_materials_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_materials_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE, task_state_buffer)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_blue_noise_image)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_gbufferGPU)
                .AddAllAttachments(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, gbufferCPU.task_images)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
                    {

                        auto push = ComputePush{
                            .gbuffer = renderer->GetDeviceAddress(ti, task_gbufferGPU),
                            .chunk_occupancy_ptr = renderer->GetDeviceAddress(ti, task_chunk_occupancy_buffer),
                            .brick_occupancy_ptr = renderer->GetDeviceAddress(ti, task_brick_occupancy_buffer),
                            .materials_ptr = renderer->GetDeviceAddress(ti, task_materials_buffer),
                            .voxel_materials_ptr = renderer->GetDeviceAddress(ti, task_voxel_materials_buffer),
                            .voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_voxel_hashmap_buffer),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer),
                            .blueNoise = blue_noise_images[stateData.frame % 64].default_view(),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}};

                        ti.recorder.set_pipeline(*render_gbuffer_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(renderer->surface_width + 31) / 32, (renderer->surface_height + 31) / 32});
                    }));

        // SSAO pass
        renderer->AddTask(
            InlineTask("ssao")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_position)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_normal)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT, gbufferCPU.task_ssao)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_blue_noise_image)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_gbufferGPU)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush{
                            .gbuffer = renderer->GetDeviceAddress(ti, task_gbufferGPU),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer),
                            .blueNoise = blue_noise_images[stateData.frame % 64].default_view(),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}
                        };

                        ti.recorder.set_pipeline(*ssao_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(renderer->surface_width / 2 + 7) / 8, (renderer->surface_height / 2 + 7) / 8});
                    }));

        // denoise gi
        renderer->AddTask(
            InlineTask("denoise_gi")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT, gbufferCPU.task_indirect)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT, gbufferCPU.task_indirectDenoised)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_depthHalfRes)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_normal)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_position)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_voxelIDs)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_gbufferGPU)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        auto push = DenoisePush
                        {
                            .gbuffer = renderer->GetDeviceAddress(ti, task_gbufferGPU),
                            .voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_voxel_hashmap_buffer),
                            .pass = 1,
                            .frame_dim = {renderer->surface_width, renderer->surface_height}
                        };

                        ti.recorder.set_pipeline(*denoise_gi_compute);

                        for (int i = 1; i <= 5; i++)
                        {
                            push.pass = i;
                            ti.recorder.push_constant(push);
                            ti.recorder.dispatch({(renderer->surface_width / LIGHTING_DOWNSAMPLE + 15) / 16, (renderer->surface_height / LIGHTING_DOWNSAMPLE + 15) / 16});
                        }
                    }));

        // combine gi
        renderer->AddTask(
            InlineTask("combine_gi")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_indirectDenoised)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT, gbufferCPU.task_indirectPerVoxelPass1)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT, gbufferCPU.task_indirectPerVoxelPass2)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_voxelIDs)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_voxelFaceIDs)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_gbufferGPU)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush{
                            .gbuffer = renderer->GetDeviceAddress(ti, task_gbufferGPU),
                            .voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_voxel_hashmap_buffer),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}};

                        ti.recorder.set_pipeline(*combine_gi_compute);

                        push.pass = 1;
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(renderer->surface_width / LIGHTING_DOWNSAMPLE + 7) / 8, (renderer->surface_height / LIGHTING_DOWNSAMPLE + 7) / 8});

                        push.pass = 2;
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(renderer->surface_width / LIGHTING_DOWNSAMPLE + 7) / 8, (renderer->surface_height / LIGHTING_DOWNSAMPLE + 7) / 8});

                        push.pass = 3;
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(renderer->surface_width / LIGHTING_DOWNSAMPLE + 7) / 8, (renderer->surface_height / LIGHTING_DOWNSAMPLE + 7) / 8});
                    }));

        renderer->AddTask(InlineTask("composite_render")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, *task_render_image)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_materials_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE, task_state_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_gbufferGPU)
                .AddAllAttachments(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_images)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush{
                            .gbuffer = renderer->GetDeviceAddress(ti, task_gbufferGPU),
                            .materials_ptr = renderer->GetDeviceAddress(ti, task_materials_buffer),
                            .voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_voxel_hashmap_buffer),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer),
                            .final_image = render_image->default_view(),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}};

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
