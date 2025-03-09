#pragma once

#include "shaders/shared.inl"
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <engine/data/Image.hpp>
#include <engine/apis/Math.hpp>
#include <engine/apis/Daxa.hpp>
#include <engine/objects/Renderer.hpp>

using namespace daxa::types;


class VoxelRenderer
{
        std::shared_ptr<Renderer> renderer;
        ComputePush pushData;

        std::shared_ptr<daxa::ComputePipeline> lighting_compute;
        std::shared_ptr<daxa::ComputePipeline> render_compute;
        daxa::BufferId lighting_buffer;
        daxa::BufferId render_buffer;
        daxa::TaskBuffer task_lighting_buffer;
        daxa::TaskBuffer task_render_buffer;

    public:
        float time;

        VoxelRenderer(std::shared_ptr<Renderer> renderer)
            : renderer(renderer)
        {
            lighting_compute = renderer->AddComputePipeline<ComputePush>("lighting", "lighting.slang");
            render_compute = renderer->AddComputePipeline<ComputePush>("render", "render.slang");

            renderer->CreateBuffer<Lighting>("lighting_buffer", lighting_buffer, task_lighting_buffer);
            renderer->CreateBuffer<Render>("render_buffer", render_buffer, task_render_buffer);
        }

        ~VoxelRenderer()
        {
            renderer->DestroyBuffer(lighting_buffer);
            renderer->DestroyBuffer(render_buffer);
        }

        void Update()
        {

        }

        void InitializeTasks(daxa::TaskGraph &task_graph, daxa::TaskImage *task_render_image, daxa::ImageId *render_image)
        {
            task_graph.use_persistent_image(*task_render_image);
            task_graph.use_persistent_buffer(task_lighting_buffer);
            task_graph.use_persistent_buffer(task_render_buffer);

            // create task to compute lighting
            renderer->AddTask(InlineTask("lighting")
                              .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE, task_lighting_buffer)
                              .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE, task_render_buffer)
                              .SetTask([this, render_image](daxa::TaskInterface ti)
            {
                auto push = ComputePush
                {
                    .lightingBuffer = renderer->GetDeviceAddress(ti, task_lighting_buffer),
                    .renderBuffer = renderer->GetDeviceAddress(ti, task_render_buffer),
                    .frame_dim = {renderer->surface_width, renderer->surface_height},
                    .time = time,
                };

                ti.recorder.set_pipeline(*lighting_compute);
                ti.recorder.push_constant(push);
                ti.recorder.dispatch({(WORLD_SIDE_LENGTH + 7) / 8, (WORLD_SIDE_LENGTH + 7) / 8});
            }));

            // create task to compute render
            renderer->AddTask(InlineTask("render")
                              .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_lighting_buffer)
                              .AddAttachment(daxa::TaskBufferAccess::COMPUTE_SHADER_READ, task_render_buffer)
                              .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, *task_render_image)
                              .SetTask([this, render_image](daxa::TaskInterface ti)
            {
                auto push = ComputePush
                {
                    .lightingBuffer = renderer->GetDeviceAddress(ti, task_lighting_buffer),
                    .renderBuffer = renderer->GetDeviceAddress(ti, task_render_buffer),
                    .screen = render_image->default_view(),
                    .frame_dim = {renderer->surface_width, renderer->surface_height},
                    .time = time,
                };

                ti.recorder.set_pipeline(*render_compute);
                ti.recorder.push_constant(push);
                ti.recorder.dispatch({(renderer->surface_width + 7) / 8, (renderer->surface_height + 7) / 8});
            }));

            // create task to blit render image to swapchain
            renderer->AddTask(renderer->CreateSwapchainBlitTask(*task_render_image));
        }

        void Resize(u32 width, u32 height)
        {
        }
};