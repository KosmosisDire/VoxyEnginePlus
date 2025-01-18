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
    std::shared_ptr<Renderer> renderer;

    std::shared_ptr<daxa::ComputePipeline> render_compute;
    std::shared_ptr<daxa::ComputePipeline> terrain_compute;
    daxa::BufferId chunk_occupancy_buffer;
    daxa::BufferId brick_occupancy_buffer;
    daxa::BufferId state_buffer;
    daxa::BufferId visible_bricks_buffer;
    daxa::BufferId compact_visible_buffer;
    daxa::BufferId brick_data_buffer;
    daxa::TaskBuffer task_chunk_occupancy_buffer;
    daxa::TaskBuffer task_brick_occupancy_buffer;
    daxa::TaskBuffer task_state_buffer;
    daxa::TaskBuffer task_visible_bricks_buffer;
    daxa::TaskBuffer task_compact_visible_buffer;
    daxa::TaskBuffer task_brick_data_buffer;

  public:
    RenderData stateData = {
        .sunDir = {1.0f, 0.2f, 0.0f},
        .camera = {},
        .frame = 0,
        .settings = {},
    };

    bool dirtyTerrain = true;

    VoxelRenderer(std::shared_ptr<Renderer> renderer)
        : renderer(renderer)
    {
        render_compute = renderer->AddComputePipeline<ComputePush>("voxel_raymarch", "render.slang");
        terrain_compute = renderer->AddComputePipeline<ComputePush>("terrain_gen", "terrain-gen.slang");

        int chunkLength = GRID_SIZE_CUBE;
        int brickLength = GRID_SIZE_CUBE * CHUNK_SIZE_CUBE;
        int chunkBytes = chunkLength * sizeof(u64);
        int brickBytes = brickLength * sizeof(BrickBitmask);

        // Initialize occupancy buffer
        renderer->CreateBuffer("chunk_occupancy", chunkBytes, chunk_occupancy_buffer, task_chunk_occupancy_buffer);
        renderer->CreateBuffer("brick_occupancy", brickBytes, brick_occupancy_buffer, task_brick_occupancy_buffer);
        renderer->CreateBuffer<RenderData>("state buffer", state_buffer, task_state_buffer);

        // Calculate size for visible bricks bitmap
        size_t max_visible = GRID_SIZE_CUBE * CHUNK_SIZE_CUBE;
        size_t bitmap_size = ((max_visible + 31) / 32) * sizeof(uint32_t);
        renderer->CreateBuffer("visible_bricks", 
            sizeof(uint32_t) * 4 + // Header with count
            bitmap_size,           // Bitmap for duplicate checking
            visible_bricks_buffer, task_visible_bricks_buffer);

        // Buffer for compacted indices - assume worst case all bricks visible
        renderer->CreateBuffer("compact_visible", 
            sizeof(uint32_t) * 4 + // Header with count
            sizeof(uint32_t) * max_visible, // Global brick indices
            compact_visible_buffer, task_compact_visible_buffer);
        
        // Create brick data buffer - one uint32 per possible brick
        size_t total_bricks = GRID_SIZE_CUBE * CHUNK_SIZE_CUBE;
        renderer->CreateBuffer("brick_data", sizeof(BrickData) * total_bricks, brick_data_buffer, task_brick_data_buffer);

        printf("chunk length: %d\n", chunkLength);
        printf("brick length: %d\n", brickLength);
        printf("bitmask size: %d\n", sizeof(BrickBitmask));
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
        task_graph.use_persistent_buffer(task_visible_bricks_buffer);
        task_graph.use_persistent_buffer(task_compact_visible_buffer);
        task_graph.use_persistent_buffer(task_brick_data_buffer);

        // create task to run main compute shader
        renderer->AddTask(
            InlineTask("copy_state")
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_WRITE, task_state_buffer)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        renderer->CopyToBuffer<RenderData>(ti, stateData, ti.get(task_state_buffer), 0);
                    }));

        // Clear visible bricks buffer
        renderer->AddTask(
            InlineTask("clear_visible_bricks")
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_WRITE, task_visible_bricks_buffer)
                .SetTask([this](daxa::TaskInterface ti) {
                    renderer->ClearBuffer(ti, ti.get(task_visible_bricks_buffer), 0);
                })
        );

        renderer->AddTask(
            InlineTask("terrain_gen")
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_chunk_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_brick_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
                    {
                        if (!dirtyTerrain) return;
                            
                        auto push = ComputePush{
                            .image = render_image->default_view(),
                            .chunk_occupancy_ptr = renderer->GetDeviceAddress(ti, task_chunk_occupancy_buffer, 0),
                            .brick_occupancy_ptr = renderer->GetDeviceAddress(ti, task_brick_occupancy_buffer, 0),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer, 0),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}};

                        ti.recorder.set_pipeline(*terrain_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(GRID_SIZE * CHUNK_SIZE + 3) / 4, (GRID_SIZE * CHUNK_SIZE + 3) / 4, (GRID_SIZE * CHUNK_SIZE + 3) / 4});

                        dirtyTerrain = false;
                    }));

        renderer->AddTask(
            InlineTask("voxel_render")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, *task_render_image)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_chunk_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_brick_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_brick_data_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE, task_visible_bricks_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_WRITE, task_compact_visible_buffer)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush{
                            .image = render_image->default_view(),
                            .chunk_occupancy_ptr = renderer->GetDeviceAddress(ti, task_chunk_occupancy_buffer, 0),
                            .brick_occupancy_ptr = renderer->GetDeviceAddress(ti, task_brick_occupancy_buffer, 0),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer, 0),
                            .visible_bricks_ptr = renderer->GetDeviceAddress(ti, task_visible_bricks_buffer, 0),
                            .compact_visible_ptr = renderer->GetDeviceAddress(ti, task_compact_visible_buffer, 0),
                            .brick_data_ptr = renderer->GetDeviceAddress(ti, task_brick_data_buffer, 0),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}};

                        ti.recorder.set_pipeline(*render_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(renderer->surface_width + 7) / 8, (renderer->surface_height + 7) / 8});
                    }));


        // create task to blit render image to swapchain
        renderer->AddTask(renderer->CreateSwapchainBlitTask(*task_render_image));
    }

    void Update()
    {
        // rotate sun around y axis
        stateData.sunDir = daxa_transform_normal(stateData.sunDir, daxa_mat_from_axis_angle({0.0f, 1.0f, 0.0f}, 0.01f * stateData.dt));
    }

  private:
};
