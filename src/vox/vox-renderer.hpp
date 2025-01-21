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
    daxa::ImageId depth;
    daxa::ImageId depthHalfRes;
    daxa::ImageId voxelIDs;

    daxa::TaskImage task_color;
    daxa::TaskImage task_normal;
    daxa::TaskImage task_position;
    daxa::TaskImage task_indirect;
    daxa::TaskImage task_depth;
    daxa::TaskImage task_depthHalfRes;
    daxa::TaskImage task_voxelIDs;

    inline void CreateImages(Renderer &renderer)
    {
        color = renderer.CreateRenderImage("color", &task_color, daxa::Format::R32G32B32A32_SFLOAT);
        normal = renderer.CreateRenderImage("normal", &task_normal, daxa::Format::R32G32B32A32_SFLOAT);
        position = renderer.CreateRenderImage("position", &task_position, daxa::Format::R32G32B32A32_SFLOAT);
        indirect = renderer.CreateRenderImage("indirect", &task_indirect, daxa::Format::R32G32B32A32_SFLOAT);
        depth = renderer.CreateRenderImage("depth", &task_depth, daxa::Format::R32_SFLOAT, 1, Renderer::depth_image_flags);
        depthHalfRes = renderer.CreateRenderImage("depthHalfRes", &task_depthHalfRes, daxa::Format::R32_SFLOAT, 0.5, Renderer::depth_image_flags);
        voxelIDs = renderer.CreateRenderImage("voxelIDs", &task_voxelIDs, daxa::Format::R32G32_UINT, 1.0, Renderer::transfer_image_flags);
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
        renderer.DestroyImage(depth);
        renderer.DestroyImage(depthHalfRes);
        renderer.DestroyImage(voxelIDs);
    }

    inline void UseImages(daxa::TaskGraph &task_graph)
    {
        task_graph.use_persistent_image(task_color);
        task_graph.use_persistent_image(task_normal);
        task_graph.use_persistent_image(task_position);
        task_graph.use_persistent_image(task_indirect);
        task_graph.use_persistent_image(task_depth);
        task_graph.use_persistent_image(task_depthHalfRes);
        task_graph.use_persistent_image(task_voxelIDs);
    }

    inline GBuffer GetGPUBuffer()
    {
        return {
            .color = color.default_view(),
            .normal = normal.default_view(),
            .position = position.default_view(),
            .indirect = indirect.default_view(),
            .depth = depth.default_view(),
            .depthHalfRes = depthHalfRes.default_view(),
            .voxelIDs = voxelIDs.default_view(),
        };
    }

    inline void TaskAddAll(InlineTask &task, daxa::TaskImageAccess access)
    {
        task.AddAttachment(access, task_color);
        task.AddAttachment(access, task_normal);
        task.AddAttachment(access, task_position);
        task.AddAttachment(access, task_indirect);
        task.AddAttachment(access, task_depth);
        task.AddAttachment(access, task_depthHalfRes);
        task.AddAttachment(access, task_voxelIDs);
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



  public:
    RenderData stateData = {
        .sunDir = {1.0f, 0.2f, 0.0f},
        .camera = {},
        .frame = 0,
        .settings = {},
    };

    GBufferCPU gbuffer;
    GBuffer gbufferGPU;

    bool dirtyTerrain = true;

    VoxelRenderer(std::shared_ptr<Renderer> renderer)
        : renderer(renderer)
    {
        terrain_compute = renderer->AddComputePipeline<ComputePush>("terrain_gen", "terrain-gen.slang");
        depth_prepass_compute = renderer->AddComputePipeline<ComputePush>("depth_prepass", "depth-prepass.slang");
        render_gbuffer_compute = renderer->AddComputePipeline<ComputePush>("voxel_raymarch", "render-gbuffer.slang");
        denoise_gi_compute = renderer->AddComputePipeline<ComputePush>("denoise_gi", "denoise-gi.slang");
        combine_gi_compute = renderer->AddComputePipeline<ComputePush>("combine_gi", "combine-gi.slang");
        render_composite_compute = renderer->AddComputePipeline<ComputePush>("composite", "composite.slang");

        // Initialize occupancy buffer
        renderer->CreateBuffer<ChunkOccupancy>("chunk_occupancy", chunk_occupancy_buffer, task_chunk_occupancy_buffer);
        renderer->CreateBuffer<BrickOccupancy>("brick_occupancy", brick_occupancy_buffer, task_brick_occupancy_buffer);
        renderer->CreateBuffer<RenderData>("state buffer", state_buffer, task_state_buffer);
        renderer->CreateBuffer<VoxelHashmap>("voxel hashmap buffer", voxel_hashmap_buffer, task_voxel_hashmap_buffer);
        renderer->CreateBuffer<VoxelHashmap>("past voxel hashmap buffer", past_voxel_hashmap_buffer, task_past_voxel_hashmap_buffer);

        gbuffer.CreateImages(*renderer);

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
        
        gbuffer.DestroyImages(*renderer);
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
        gbuffer.UseImages(task_graph);

        renderer->AddTask(
            InlineTask("setup_gbuffer")
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_past_voxel_hashmap_buffer)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        gbufferGPU = gbuffer.GetGPUBuffer();
                        renderer->CopyBuffer(ti, task_voxel_hashmap_buffer, task_past_voxel_hashmap_buffer);
                        renderer->ClearBuffer(ti, task_voxel_hashmap_buffer, 0);
                    }));

        renderer->AddTask(
            InlineTask("copy_state")
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_WRITE, task_state_buffer)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        renderer->CopyToBuffer<RenderData>(ti, stateData, ti.get(task_state_buffer), 0);
                    }));

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
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, gbuffer.task_depthHalfRes)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_chunk_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_brick_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush
                        {
                            .gbuffer = gbufferGPU,
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
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT, task_blue_noise_image)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
                    {

                        auto push = ComputePush{
                            .gbuffer = gbufferGPU,
                            .blueNoise = blue_noise_images[stateData.frame % 7].default_view(),
                            .chunk_occupancy_ptr = renderer->GetDeviceAddress(ti, task_chunk_occupancy_buffer),
                            .brick_occupancy_ptr = renderer->GetDeviceAddress(ti, task_brick_occupancy_buffer),
                            .voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_voxel_hashmap_buffer),
                            .past_voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_past_voxel_hashmap_buffer),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}};

                        ti.recorder.set_pipeline(*render_gbuffer_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(renderer->surface_width + 7) / 8, (renderer->surface_height + 7) / 8});
                    });
        gbuffer.TaskAddAll(main_render_gbuffer_task, daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT);
        renderer->AddTask(main_render_gbuffer_task);

        // denoise gi
        renderer->AddTask(
            InlineTask("denoise_gi")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT, gbuffer.task_indirect)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbuffer.task_depth)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbuffer.task_normal)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbuffer.task_position)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        auto push = DenoisePush
                        {
                            .gbuffer = gbufferGPU,
                            .pass = 1,
                            .frame_dim = {renderer->surface_width, renderer->surface_height
                        }};

                        ti.recorder.set_pipeline(*denoise_gi_compute);

                        for (int i = 0; i < 8; i++)
                        {
                            push.pass = i;
                            ti.recorder.push_constant(push);
                            ti.recorder.dispatch({(renderer->surface_width + 15) / 16, (renderer->surface_height + 15) / 16});
                        }
                    }));

        // combine gi
        renderer->AddTask(
            InlineTask("combine_gi")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT, gbuffer.task_indirect)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, gbuffer.task_voxelIDs)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_past_voxel_hashmap_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush{
                            .gbuffer = gbufferGPU,
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
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush{
                            .gbuffer = gbufferGPU,
                            .final_image = render_image->default_view(),
                            .voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_voxel_hashmap_buffer),
                            .past_voxel_hashmap_ptr = renderer->GetDeviceAddress(ti, task_past_voxel_hashmap_buffer),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}};

                        ti.recorder.set_pipeline(*render_composite_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(renderer->surface_width + 7) / 8, (renderer->surface_height + 7) / 8});
                    });
        gbuffer.TaskAddAll(composite_render_task, daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY);
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
        gbuffer.ResizeImages(*renderer);
    }

};
