#pragma once

#include "shaders/shared.inl"
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <math.hpp>
#include <renderer.hpp>

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
        color = renderer.CreateRenderImage("color", &task_color);
        normal = renderer.CreateRenderImage("normal", &task_normal);
        position = renderer.CreateRenderImage("position", &task_position);
        indirect = renderer.CreateRenderImage("indirect", &task_indirect);
        depth = renderer.CreateRenderImage("depth", &task_depth, daxa::Format::R32_SFLOAT, 1,  Renderer::depth_image_flags);
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

    std::shared_ptr<daxa::ComputePipeline> render_gbuffer_compute;
    std::shared_ptr<daxa::ComputePipeline> render_composite_compute;
    std::shared_ptr<daxa::ComputePipeline> terrain_compute;
    std::shared_ptr<daxa::ComputePipeline> depth_prepass_compute;
    daxa::BufferId chunk_occupancy_buffer;
    daxa::BufferId brick_occupancy_buffer;
    daxa::BufferId state_buffer;

    daxa::TaskBuffer task_chunk_occupancy_buffer;
    daxa::TaskBuffer task_brick_occupancy_buffer;
    daxa::TaskBuffer task_state_buffer;

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
        render_gbuffer_compute = renderer->AddComputePipeline<ComputePush>("voxel_raymarch", "render-gbuffer.slang");
        render_composite_compute = renderer->AddComputePipeline<ComputePush>("composite", "composite.slang");
        terrain_compute = renderer->AddComputePipeline<ComputePush>("terrain_gen", "terrain-gen.slang");
        depth_prepass_compute = renderer->AddComputePipeline<ComputePush>("depth_prepass", "depth-prepass.slang");

        int chunkLength = GRID_SIZE_CUBE;
        int brickLength = GRID_SIZE_CUBE * CHUNK_SIZE_CUBE;
        int chunkBytes = chunkLength * sizeof(u64);
        int brickBytes = brickLength * sizeof(BrickBitmask);

        // Initialize occupancy buffer
        renderer->CreateBuffer("chunk_occupancy", chunkBytes, chunk_occupancy_buffer, task_chunk_occupancy_buffer);
        renderer->CreateBuffer("brick_occupancy", brickBytes, brick_occupancy_buffer, task_brick_occupancy_buffer);
        renderer->CreateBuffer<RenderData>("state buffer", state_buffer, task_state_buffer);

        gbuffer.CreateImages(*renderer);   
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
        task_graph.use_persistent_buffer(task_state_buffer);

        gbuffer.UseImages(task_graph);

        renderer->AddTask(
            InlineTask("setup_gbuffer")
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        gbufferGPU = gbuffer.GetGPUBuffer();
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
                            .chunk_occupancy_ptr = renderer->GetDeviceAddress(ti, task_chunk_occupancy_buffer, 0),
                            .brick_occupancy_ptr = renderer->GetDeviceAddress(ti, task_brick_occupancy_buffer, 0),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer, 0),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}
                        };

                        ti.recorder.set_pipeline(*depth_prepass_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({((renderer->surface_width / 2) + 7) / 8, ((renderer->surface_height / 2) + 7) / 8});
                    }));

        
        auto main_render_gbuffer_task = InlineTask("main_render_gbuffer") 
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_chunk_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_brick_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush{
                            .gbuffer = gbufferGPU,
                            .chunk_occupancy_ptr = renderer->GetDeviceAddress(ti, task_chunk_occupancy_buffer, 0),
                            .brick_occupancy_ptr = renderer->GetDeviceAddress(ti, task_brick_occupancy_buffer, 0),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer, 0),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}};

                        ti.recorder.set_pipeline(*render_gbuffer_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(renderer->surface_width + 7) / 8, (renderer->surface_height + 7) / 8});
                    });
        gbuffer.TaskAddAll(main_render_gbuffer_task, daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT);
        renderer->AddTask(main_render_gbuffer_task);


        auto composite_render_task = InlineTask("composite_render")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, *task_render_image)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush{
                            .gbuffer = gbufferGPU,
                            .final_image = render_image->default_view(),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer, 0),
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
        stateData.sunDir = daxa_transform_normal(stateData.sunDir, daxa_mat_from_axis_angle({0.0f, 1.0f, 0.0f}, 0.01f * stateData.dt));
    }

    void Resize(u32 width, u32 height)
    {
        gbuffer.ResizeImages(*renderer);
    }

};
