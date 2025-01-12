#pragma once
#include "shaders/shared.inl"
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <math.hpp>
#include <renderer.hpp>

using namespace daxa::types;

class VoxelRenderer
{
    Renderer *renderer;

    std::shared_ptr<daxa::ComputePipeline> render_compute;
    std::shared_ptr<daxa::ComputePipeline> terrain_compute;
    daxa::BufferId chunk_occupancy_buffer;
    daxa::BufferId brick_occupancy_buffer;
    daxa::BufferId state_buffer;
    daxa::TaskBuffer task_chunk_occupancy_buffer;
    daxa::TaskBuffer task_brick_occupancy_buffer;
    daxa::TaskBuffer task_state_buffer;

  public:
    StateData stateData = {
        .sunDir = {1.0f, 0.2f, 0.0f},
        .camera = {},
    };

    VoxelRenderer(Renderer *renderer)
        : renderer(renderer)
    {
        render_compute = renderer->AddComputePipeline<ComputePush>("voxel_raymarch", "compute.slang");
        terrain_compute = renderer->AddComputePipeline<ComputePush>("terrain_gen", "terrain-gen.slang");

        // Initialize occupancy buffer
        renderer->CreateBuffer("chunk_occupancy", TREE_SIZE_CUBE / BITS_PER_BYTE, chunk_occupancy_buffer, task_chunk_occupancy_buffer);
        renderer->CreateBuffer("brick_occupancy", (TREE_SIZE_CUBE * TREE_SIZE_CUBE) / BITS_PER_BYTE, brick_occupancy_buffer, task_brick_occupancy_buffer);
        renderer->CreateBuffer<StateData>("state buffer", state_buffer, task_state_buffer);

        auto chunk_ptr = renderer->MapBufferAs<Occupancy>(chunk_occupancy_buffer);
        auto brick_ptr = renderer->MapBufferAs<Occupancy>(brick_occupancy_buffer);

        // Initialize chunk occupancy buffer
        for (usize i = 0; i < TREE_SIZE_CUBE / BITS_PER_BYTE / sizeof(u64); i++)
        {
            chunk_ptr->occupancy[i] = 0;
        }

        // Initialize brick occupancy buffer
        for (usize i = 0; i < (TREE_SIZE_CUBE * TREE_SIZE_CUBE) / BITS_PER_BYTE / sizeof(u64); i++)
        {
            brick_ptr->occupancy[i] = 0;
        }
    }

    ~VoxelRenderer()
    {
        renderer->DestroyBuffer(chunk_occupancy_buffer);
        renderer->DestroyBuffer(brick_occupancy_buffer);
        renderer->DestroyBuffer(state_buffer);
    }

    void InitializeTasks(daxa::TaskGraph &task_graph, daxa::TaskImage *task_render_image, daxa::ImageId *render_image)
    {
        task_graph.use_persistent_image(*task_render_image);
        task_graph.use_persistent_buffer(task_chunk_occupancy_buffer);
        task_graph.use_persistent_buffer(task_brick_occupancy_buffer);
        task_graph.use_persistent_buffer(task_state_buffer);

        // create task to run main compute shader
        renderer->AddTask(
            InlineTask("voxel_render")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, *task_render_image)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_chunk_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_brick_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_state_buffer)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
                    {
                        printf("Width: %d, Height: %d\n", renderer->surface_width, renderer->surface_height);
                        renderer->CopyToBuffer<StateData>(ti, stateData, ti.get(task_state_buffer), 0);
                        auto push = ComputePush{
                            .image = render_image->default_view(),
                            .chunk_occupancy_ptr = renderer->GetDeviceAddress(ti, task_chunk_occupancy_buffer, 0),
                            .brick_occupancy_ptr = renderer->GetDeviceAddress(ti, task_brick_occupancy_buffer, 0),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer, 0),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}};

                        ti.recorder.set_pipeline(*terrain_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(256 + 7) / 8, (156 + 7) / 8, (256 + 7) / 8});

                        ti.recorder.set_pipeline(*render_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(renderer->surface_width + 7) / 8, (renderer->surface_height + 7) / 8});
                    }));

        // create task to blit render image to swapchain
        renderer->AddTask(renderer->CreateSwapchainBlitTask(*task_render_image));
    }

    void Update(StateData state)
    {
        stateData = state;
        // rotate sun around y axis
        stateData.sunDir = daxa_transform_normal(stateData.sunDir, daxa_mat_from_axis_angle({0.0f, 1.0f, 0.0f}, 0.01f * state.dt));
    }

  private:
};