#pragma once

#include "shaders/shared.inl"
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <math.hpp>
#include <renderer.hpp>
#include <image.hpp>

using namespace daxa::types;


struct GBufferCPU
{
    daxa::ImageId color;
    daxa::ImageId normal;
    daxa::ImageId position;
    daxa::ImageId indirect;
    daxa::ImageId indirectLast;
    daxa::ImageId indirectDenoised;
    daxa::ImageId motion;
    daxa::ImageId depth;
    daxa::ImageId depthHalfRes;
    daxa::ImageId voxelIDs;
    daxa::ImageId ssao;

    daxa::TaskImage task_color;
    daxa::TaskImage task_normal;
    daxa::TaskImage task_position;
    daxa::TaskImage task_indirect;
    daxa::TaskImage task_indirectLast;
    daxa::TaskImage task_indirectDenoised;
    daxa::TaskImage task_motion;
    daxa::TaskImage task_depth;
    daxa::TaskImage task_depthHalfRes;
    daxa::TaskImage task_voxelIDs;
    daxa::TaskImage task_ssao;

    inline void CreateImages(Renderer &renderer)
    {
        color = renderer.CreateRenderImage("color", &task_color, daxa::Format::R16G16B16A16_SFLOAT);
        normal = renderer.CreateRenderImage("normal", &task_normal, daxa::Format::R8G8B8A8_SINT);
        position = renderer.CreateRenderImage("position", &task_position, daxa::Format::R32G32B32A32_SFLOAT);
        indirect = renderer.CreateRenderImage("indirect", &task_indirect, daxa::Format::R16G16B16A16_SFLOAT, 0.5);
        indirectLast = renderer.CreateRenderImage("indirectLast", &task_indirectLast, daxa::Format::R16G16B16A16_SFLOAT, 0.5);
        indirectDenoised = renderer.CreateRenderImage("indirectDenoised", &task_indirectDenoised, daxa::Format::R16G16B16A16_SFLOAT, 0.5);
        motion = renderer.CreateRenderImage("motion", &task_motion, daxa::Format::R16G16_SFLOAT);
        depth = renderer.CreateRenderImage("depth", &task_depth, daxa::Format::R32_SFLOAT, 1);
        depthHalfRes = renderer.CreateRenderImage("depthHalfRes", &task_depthHalfRes, daxa::Format::R32_SFLOAT, 0.5);
        voxelIDs = renderer.CreateRenderImage("voxelIDs", &task_voxelIDs, daxa::Format::R32G32_UINT, 1.0);
        ssao = renderer.CreateRenderImage("ssao", &task_ssao, daxa::Format::R32_SFLOAT, 0.5);
    }

    inline void ResizeImages(Renderer &renderer)
    {
        DestroyImages(renderer);
        CreateImages(renderer);
    }

    inline void DestroyImages(Renderer &renderer)
    {
        renderer.DestroyImage(color);
        renderer.DestroyImage(normal);
        renderer.DestroyImage(position);
        renderer.DestroyImage(indirect);
        renderer.DestroyImage(indirectLast);
        renderer.DestroyImage(indirectDenoised);
        renderer.DestroyImage(motion);
        renderer.DestroyImage(depth);
        renderer.DestroyImage(depthHalfRes);
        renderer.DestroyImage(voxelIDs);
        renderer.DestroyImage(ssao);
    }

    inline void UseImages(daxa::TaskGraph &task_graph)
    {
        task_graph.use_persistent_image(task_color);
        task_graph.use_persistent_image(task_normal);
        task_graph.use_persistent_image(task_position);
        task_graph.use_persistent_image(task_indirect);
        task_graph.use_persistent_image(task_indirectLast);
        task_graph.use_persistent_image(task_indirectDenoised);
        task_graph.use_persistent_image(task_motion);
        task_graph.use_persistent_image(task_depth);
        task_graph.use_persistent_image(task_depthHalfRes);
        task_graph.use_persistent_image(task_voxelIDs);
        task_graph.use_persistent_image(task_ssao);
    }

    inline GBuffer GetGPUBuffer(int frame)
    {
        return {
            .color = color.default_view(),
            .normal = normal.default_view(),
            .position = position.default_view(),
            .indirect = frame % 2 == 0 ? indirect.default_view() : indirectLast.default_view(),
            .indirectLast = frame % 2 == 0 ? indirectLast.default_view() : indirect.default_view(),
            .indirectDenoised = indirectDenoised.default_view(),
            .motion = motion.default_view(),
            .depth = depth.default_view(),
            .depthHalfRes = depthHalfRes.default_view(),
            .voxelIDs = voxelIDs.default_view(),
            .ssao = ssao.default_view(),
        };
    }

    inline void TaskAddAll(InlineTask &task, daxa::TaskImageAccess access)
    {
        task.AddAttachment(access, task_color);
        task.AddAttachment(access, task_normal);
        task.AddAttachment(access, task_position);
        task.AddAttachment(access, task_indirect);
        task.AddAttachment(access, task_indirectLast);
        task.AddAttachment(access, task_indirectDenoised);
        task.AddAttachment(access, task_motion);
        task.AddAttachment(access, task_depth);
        task.AddAttachment(access, task_depthHalfRes);
        task.AddAttachment(access, task_voxelIDs);
        task.AddAttachment(access, task_ssao);
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

    daxa::BufferId state_buffer;
    daxa::TaskBuffer task_state_buffer;

    daxa::BufferId voxel_hashmap_buffer;
    daxa::TaskBuffer task_voxel_hashmap_buffer;

    daxa::BufferId past_voxel_hashmap_buffer; // from last frame
    daxa::TaskBuffer task_past_voxel_hashmap_buffer; // from last frame

    std::unique_ptr<defect::Image> blue_noise_images_data[64];
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
        renderer->CreateBuffer<RenderData>("state buffer", state_buffer, task_state_buffer);
        renderer->CreateBuffer<VoxelHashmap>("voxel hashmap buffer", voxel_hashmap_buffer, task_voxel_hashmap_buffer);
        renderer->CreateBuffer<VoxelHashmap>("past voxel hashmap buffer", past_voxel_hashmap_buffer, task_past_voxel_hashmap_buffer);
        renderer->CreateBuffer<GBuffer>("gbufferGPU", gbufferGPU, task_gbufferGPU);

        gbufferCPU.CreateImages(*renderer);

        // load blue noise images
        for (int i = 0; i < 64; i++)
        {
            auto filename = std::to_string(i) + ".png";
            auto image = std::make_unique<defect::Image>("resources/textures/noise/vec3/" + filename);
            blue_noise_images[i] = renderer->CreateImage(filename, *image);
            blue_noise_images_data[i] = std::move(image);
        }

        // create noise task image
        task_blue_noise_image = daxa::TaskImage(daxa::TaskImageInfo{.initial_images = {.images = blue_noise_images}, .name = "task_blue_noise_image"});
    }

    ~VoxelRenderer()
    {
        renderer->DestroyBuffer(chunk_occupancy_buffer);
        renderer->DestroyBuffer(brick_occupancy_buffer);
        renderer->DestroyBuffer(state_buffer);
        renderer->DestroyBuffer(voxel_hashmap_buffer);
        renderer->DestroyBuffer(past_voxel_hashmap_buffer);

        for (int i = 0; i < 64; i++)
        {
            renderer->DestroyImage(blue_noise_images[i]);
        }
        
        gbufferCPU.DestroyImages(*renderer);
    }

    void InitializeTasks(daxa::TaskGraph &task_graph, daxa::TaskImage *task_render_image, daxa::ImageId *render_image)
    {
        task_graph.use_persistent_image(*task_render_image);
        task_graph.use_persistent_buffer(task_chunk_occupancy_buffer);
        task_graph.use_persistent_buffer(task_brick_occupancy_buffer);
        task_graph.use_persistent_buffer(task_voxel_hashmap_buffer);
        task_graph.use_persistent_buffer(task_past_voxel_hashmap_buffer);
        task_graph.use_persistent_buffer(task_state_buffer);
        task_graph.use_persistent_image(task_blue_noise_image);
        task_graph.use_persistent_buffer(task_gbufferGPU); 
        gbufferCPU.UseImages(task_graph);

        auto setup_gbuffer_task =
            InlineTask("setup_gbuffer")
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_past_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_WRITE, task_state_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_WRITE, task_gbufferGPU)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        renderer->CopyToBuffer<GBuffer>(ti, gbufferCPU.GetGPUBuffer(stateData.frame), ti.get(task_gbufferGPU), 0);
                        
                        stateData.lastCamera = stateData.camera;
                        stateData.camera = camera.getCameraData();
                        renderer->CopyToBuffer<RenderData>(ti, stateData, ti.get(task_state_buffer), 0);

                        auto temp = task_voxel_hashmap_buffer;
                        task_voxel_hashmap_buffer = task_past_voxel_hashmap_buffer; 
                        task_past_voxel_hashmap_buffer = temp;

                        renderer->ClearBuffer(ti, task_voxel_hashmap_buffer, 0);
                    });
        gbufferCPU.TaskAddAll(setup_gbuffer_task, daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT);
        renderer->AddTask(setup_gbuffer_task);

        renderer->AddTask(
            InlineTask("terrain_gen")
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_chunk_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_brick_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
                    {
                        if (!dirtyTerrain) return;
                            
                        auto push = ComputePush
                        {
                            .chunk_occupancy_ptr = renderer->GetDeviceAddress(ti, task_chunk_occupancy_buffer, 0),
                            .brick_occupancy_ptr = renderer->GetDeviceAddress(ti, task_brick_occupancy_buffer, 0),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer, 0)
                        };

                        ti.recorder.set_pipeline(*terrain_compute);
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

        
        auto main_render_gbuffer_task = InlineTask("main_render_gbuffer") 
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_chunk_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_brick_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_past_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE, task_state_buffer)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT, task_blue_noise_image)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_gbufferGPU)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
                    {

                        auto push = ComputePush{
                            .gbuffer = renderer->GetDeviceAddress(ti, task_gbufferGPU),
                            .chunk_occupancy_ptr = renderer->GetDeviceAddress(ti, task_chunk_occupancy_buffer),
                            .brick_occupancy_ptr = renderer->GetDeviceAddress(ti, task_brick_occupancy_buffer),
                            .voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_voxel_hashmap_buffer),
                            .past_voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_past_voxel_hashmap_buffer),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer),
                            .blueNoise = blue_noise_images[stateData.frame % 64].default_view(),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}};

                        ti.recorder.set_pipeline(*render_gbuffer_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(renderer->surface_width + 15) / 16, (renderer->surface_height + 15) / 16});
                    });
        gbufferCPU.TaskAddAll(main_render_gbuffer_task, daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT);
        renderer->AddTask(main_render_gbuffer_task);

        // SSAO pass
        renderer->AddTask(
            InlineTask("ssao")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_position)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_normal)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT, gbufferCPU.task_ssao)
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
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_past_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_gbufferGPU)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        auto push = DenoisePush
                        {
                            .gbuffer = renderer->GetDeviceAddress(ti, task_gbufferGPU),
                            .voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_voxel_hashmap_buffer),
                            .past_voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_past_voxel_hashmap_buffer),
                            .pass = 1,
                            .frame_dim = {renderer->surface_width, renderer->surface_height}
                        };

                        ti.recorder.set_pipeline(*denoise_gi_compute);

                        for (int i = 1; i <= 8; i++)
                        {
                            push.pass = i;
                            ti.recorder.push_constant(push);
                            ti.recorder.dispatch({(renderer->surface_width / 2 + 15) / 16, (renderer->surface_height / 2 + 15) / 16});
                        }
                    }));

        // combine gi
        renderer->AddTask(
            InlineTask("combine_gi")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_indirectDenoised)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbufferCPU.task_voxelIDs)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_past_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_gbufferGPU)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush{
                            .gbuffer = renderer->GetDeviceAddress(ti, task_gbufferGPU),
                            .voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_voxel_hashmap_buffer),
                            .past_voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_past_voxel_hashmap_buffer),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}};

                        ti.recorder.set_pipeline(*combine_gi_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(renderer->surface_width + 7) / 8, (renderer->surface_height + 7) / 8});
                    }));


        auto composite_render_task = InlineTask("composite_render")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, *task_render_image)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_past_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE, task_state_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_gbufferGPU)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush{
                            .gbuffer = renderer->GetDeviceAddress(ti, task_gbufferGPU),
                            .voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_voxel_hashmap_buffer),
                            .past_voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_past_voxel_hashmap_buffer),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer),
                            .final_image = render_image->default_view(),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}};

                        ti.recorder.set_pipeline(*render_composite_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(renderer->surface_width + 7) / 8, (renderer->surface_height + 7) / 8});
                    });
        gbufferCPU.TaskAddAll(composite_render_task, daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY);
        renderer->AddTask(composite_render_task);

        // create task to blit render image to swapchain
        renderer->AddTask(renderer->CreateSwapchainBlitTask(*task_render_image));
    }

    void Update()
    {
        // rotate sun around y axis
        // stateData.sunDir = daxa_transform_normal(stateData.sunDir, daxa_mat_from_axis_angle({0.0f, 1.0f, 0.0f}, 0.01f * stateData.dt));
    }

    void Resize(u32 width, u32 height)
    {
        gbufferCPU.ResizeImages(*renderer);
        camera.setViewportSize(width, height);
    }

};
