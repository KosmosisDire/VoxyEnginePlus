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
    std::shared_ptr<daxa::ComputePipeline> depth_prepass_compute;
    std::shared_ptr<daxa::ComputePipeline> global_illumination_compute;
    std::shared_ptr<daxa::ComputePipeline> gi_average_compute;
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

    daxa::ImageId depth_prepass_image;
    daxa::TaskImage task_depth_prepass_image;
    daxa::ImageId pixel_to_brick_image;
    daxa::TaskImage task_pixel_to_brick_image;

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
        depth_prepass_compute = renderer->AddComputePipeline<ComputePush>("depth_prepass", "depth-prepass.slang");
        global_illumination_compute = renderer->AddComputePipeline<ComputePush>("brick_global_illumination", "global-illumination.slang");
        gi_average_compute = renderer->AddComputePipeline<ComputePush>("gi_average", "gi-average.slang");

        int chunkLength = GRID_SIZE_CUBE;
        int brickLength = GRID_SIZE_CUBE * CHUNK_SIZE_CUBE;
        int chunkBytes = chunkLength * sizeof(u64);
        int brickBytes = brickLength * sizeof(BrickBitmask);

        // Initialize occupancy buffer
        renderer->CreateBuffer("chunk_occupancy", chunkBytes, chunk_occupancy_buffer, task_chunk_occupancy_buffer);
        renderer->CreateBuffer("brick_occupancy", brickBytes, brick_occupancy_buffer, task_brick_occupancy_buffer);
        renderer->CreateBuffer<RenderData>("state buffer", state_buffer, task_state_buffer);

        // Calculate size for visible bricks bitmap
        size_t max_visible = GRID_SIZE_CUBE * CHUNK_SIZE_CUBE * 0.5;
        renderer->CreateBuffer("visible_bricks", chunkBytes, visible_bricks_buffer, task_visible_bricks_buffer);

        // Buffer for compacted indices
        renderer->CreateBuffer("compact_visible", sizeof(uint32_t) * max_visible, compact_visible_buffer, task_compact_visible_buffer);
        
        // Create brick data buffer
        renderer->CreateBuffer("brick_data", sizeof(BrickData), brick_data_buffer, task_brick_data_buffer);

        // create depth prepass image
        depth_prepass_image = renderer->CreateImage(daxa::ImageInfo{
            .format = daxa::Format::R32_SFLOAT,
            .size = {renderer->surface_width / 2, renderer->surface_height / 2, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .name = "depth_prepass_image",
        }, task_depth_prepass_image);

        // create pixel to brick image
        pixel_to_brick_image = renderer->CreateImage(daxa::ImageInfo{
            .format = daxa::Format::R32_UINT,
            .size = {renderer->surface_width / 2, renderer->surface_height / 2, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_DST,
            .name = "pixel_to_brick_image",
        }, task_pixel_to_brick_image);
    }

    ~VoxelRenderer()
    {
        renderer->DestroyBuffer(chunk_occupancy_buffer);
        renderer->DestroyBuffer(brick_occupancy_buffer);
        renderer->DestroyBuffer(state_buffer);
        renderer->DestroyBuffer(visible_bricks_buffer);
        renderer->DestroyBuffer(compact_visible_buffer);
        renderer->DestroyBuffer(brick_data_buffer);
        renderer->DestroyImage(depth_prepass_image);
        renderer->DestroyImage(pixel_to_brick_image);
    }

    void InitializeTasks(daxa::TaskGraph &task_graph, daxa::TaskImage *task_render_image, daxa::ImageId *render_image)
    {
        task_graph.use_persistent_image(*task_render_image);
        task_graph.use_persistent_image(task_depth_prepass_image);
        task_graph.use_persistent_image(task_pixel_to_brick_image);
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
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_WRITE, task_compact_visible_buffer)
                .SetTask([this](daxa::TaskInterface ti) {
                    renderer->ClearBuffer(ti, ti.get(task_visible_bricks_buffer), 0);
                    renderer->ClearBuffer(ti, ti.get(task_compact_visible_buffer), 0);
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
                            .final_image = render_image->default_view(),
                            .chunk_occupancy_ptr = renderer->GetDeviceAddress(ti, task_chunk_occupancy_buffer, 0),
                            .brick_occupancy_ptr = renderer->GetDeviceAddress(ti, task_brick_occupancy_buffer, 0),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer, 0),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}};

                        ti.recorder.set_pipeline(*terrain_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(GRID_SIZE * CHUNK_SIZE + 3) / 4, (GRID_SIZE * CHUNK_SIZE + 3) / 4, (GRID_SIZE * CHUNK_SIZE + 3) / 4});

                        dirtyTerrain = false;
                    }));

        // depth prepass
        renderer->AddTask(
            InlineTask("depth_prepass")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_depth_prepass_image)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_pixel_to_brick_image)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_chunk_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_brick_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_visible_bricks_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, task_compact_visible_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush
                        {
                            .depth_prepass = depth_prepass_image.default_view(),
                            .pixel_to_brick = pixel_to_brick_image.default_view(),
                            .chunk_occupancy_ptr = renderer->GetDeviceAddress(ti, task_chunk_occupancy_buffer, 0),
                            .brick_occupancy_ptr = renderer->GetDeviceAddress(ti, task_brick_occupancy_buffer, 0),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer, 0),
                            .visible_bricks_ptr = renderer->GetDeviceAddress(ti, task_visible_bricks_buffer, 0),
                            .compact_visible_ptr = renderer->GetDeviceAddress(ti, task_compact_visible_buffer, 0),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}
                        };

                        ti.recorder.set_pipeline(*depth_prepass_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({((renderer->surface_width / 2) + 7) / 8, ((renderer->surface_height / 2) + 7) / 8});
                    }));

        // global illumination
        renderer->AddTask(
            InlineTask("global_illumination")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_pixel_to_brick_image)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_chunk_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_brick_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE, task_brick_data_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_compact_visible_buffer)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush{
                            .pixel_to_brick = pixel_to_brick_image.default_view(),
                            .chunk_occupancy_ptr = renderer->GetDeviceAddress(ti, task_chunk_occupancy_buffer, 0),
                            .brick_occupancy_ptr = renderer->GetDeviceAddress(ti, task_brick_occupancy_buffer, 0),
                            .state_ptr = renderer->GetDeviceAddress(ti, task_state_buffer, 0),
                            .compact_visible_ptr = renderer->GetDeviceAddress(ti, task_compact_visible_buffer, 0),
                            .brick_data_ptr = renderer->GetDeviceAddress(ti, task_brick_data_buffer, 0),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}};

                        ti.recorder.set_pipeline(*global_illumination_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(renderer->surface_width + 7) / 8, (renderer->surface_height + 7) / 8});
                    }));

        // GI averaging pass
        renderer->AddTask(
            InlineTask("gi_average")
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE, task_brick_data_buffer)
                .SetTask(
                    [this](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush{
                            .brick_data_ptr = renderer->GetDeviceAddress(ti, task_brick_data_buffer, 0),
                            .frame_dim = {renderer->surface_width, renderer->surface_height}
                        };

                        ti.recorder.set_pipeline(*gi_average_compute);
                        ti.recorder.push_constant(push);
                        ti.recorder.dispatch({(GRID_SIZE_CUBE * CHUNK_SIZE_CUBE + 255) / 256, 1, 1});
                    }));

        renderer->AddTask(
            InlineTask("voxel_render")
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, *task_render_image)
                .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_depth_prepass_image)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_chunk_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_brick_occupancy_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_state_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_brick_data_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_visible_bricks_buffer)
                .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_compact_visible_buffer)
                .SetTask(
                    [this, render_image](daxa::TaskInterface ti)
                    {
                        auto push = ComputePush{
                            .final_image = render_image->default_view(),
                            .depth_prepass = depth_prepass_image.default_view(),
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

    void Resize(u32 width, u32 height)
    {
        renderer->DestroyImage(depth_prepass_image);
        depth_prepass_image = renderer->CreateImage(daxa::ImageInfo{
            .format = daxa::Format::R32_SFLOAT,
            .size = {width / 2, height / 2, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .name = "depth_prepass_image",
        });
        task_depth_prepass_image.set_images({.images = std::array{depth_prepass_image}});

        renderer->DestroyImage(pixel_to_brick_image);
        pixel_to_brick_image = renderer->CreateImage(daxa::ImageInfo{
            .format = daxa::Format::R32_UINT,
            .size = {width / 2, height / 2, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_DST,
            .name = "pixel_to_brick_image",
        });
        task_pixel_to_brick_image.set_images({.images = std::array{pixel_to_brick_image}});
    }

};
