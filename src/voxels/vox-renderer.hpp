#pragma once

#include "shaders/shared.inl"
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <engine/data/Image.hpp>
#include <engine/apis/Math.hpp>
#include <engine/apis/Daxa.hpp>
#include <engine/objects/Renderer.hpp>
#include "tree-shapes.hpp"
#include "tree64.hpp"

using namespace daxa::types;

class VoxelRenderer
{
        std::shared_ptr<Renderer> renderer;
        ComputePush pushData;
        Camera camera;

        std::shared_ptr<daxa::ComputePipeline> render_compute;

        BufferId nodes;
        daxa::TaskBuffer task_nodes;
        BufferId data;
        daxa::TaskBuffer task_data;

        size_t node_buffer_capacity;
        size_t data_buffer_capacity;
        int world_size;

    public:
        float time;
        daxa_u32 frame;

        VoxelRenderer(std::shared_ptr<Renderer> renderer)
            : renderer(renderer)
        {
            render_compute = renderer->AddComputePipeline<ComputePush>("render", "render.slang");
            scriptingEngine.registerProperty("Camera camera", &camera);
            frame = 0;
            world_size = 8; // Store this for regeneration

            // Ensure buffer sizes match expected node structure
            const size_t node_size = sizeof(RawNode); // Should be exactly 12 bytes (3 uint32)
            const size_t data_size = sizeof(uint8_t); // Should be exactly 1 byte

            // Create a sphere shape
            int powSize = 1 << world_size;
            Sphere sphere(Vec3(powSize / 2, powSize / 2, powSize / 2), powSize / 2 - 4);
            Perlin perlin(0.5f, 42, 50.0f, 4);
            CompositeShape shape({&sphere, &perlin}, BooleanOperation::DIFFERENCE);

            Tree64 tree(world_size);
            tree.InitializeFromShape(shape);
            auto octree = tree.GetOctreeData();

            // Allocate with extra capacity for future regenerations
            // For a size 8 octree, we allocate 1.5x the initial size to handle variations
            node_buffer_capacity = octree.nodes.size() * 1.5;
            data_buffer_capacity = octree.leafData.size() * 1.5;

            renderer->CreateBuffer("nodes", node_buffer_capacity * node_size, nodes, task_nodes, daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE);
            void* mapped_nodes = renderer->MapBufferAs<void>(nodes);
            std::memcpy(mapped_nodes, octree.nodes.data(), octree.nodes.size() * node_size);

            renderer->CreateBuffer("data", data_buffer_capacity * data_size, data, task_data, daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE);
            void* mapped_data = renderer->MapBufferAs<void>(data);
            std::memcpy(mapped_data, octree.leafData.data(), octree.leafData.size() * data_size);
        }


        ~VoxelRenderer()
        {
            renderer->DestroyBuffer(nodes);
            renderer->DestroyBuffer(data);
        }

        void RegenerateWorld()
        {
            // Create a new shape with different perlin seed
            int powSize = 1 << world_size;
            Sphere sphere(Vec3(powSize / 2, powSize / 2, powSize / 2), powSize / 2 - 4);

            // Use frame number as a changing seed for variation
            Perlin perlin(0.5f, frame * 17 + 42, 50.0f, 4);
            CompositeShape shape({&sphere, &perlin}, BooleanOperation::DIFFERENCE);

            // Rebuild the tree
            Tree64 tree(world_size);
            tree.InitializeFromShape(shape);
            auto octree = tree.GetOctreeData();

            // Ensure buffer sizes match expected node structure
            const size_t node_size = sizeof(RawNode);
            const size_t data_size = sizeof(uint8_t);

            // Check if the new data will fit in our preallocated buffers
            if (octree.nodes.size() > node_buffer_capacity || octree.leafData.size() > data_buffer_capacity)
            {
                // Log warning - would need to resize buffers which would invalidate task graph
                // In a real implementation, you might want to handle this case more gracefully
                return;
            }

            // Update the node buffer
            void* mapped_nodes = renderer->MapBufferAs<void>(nodes);
            std::memcpy(mapped_nodes, octree.nodes.data(), octree.nodes.size() * node_size);

            // Update the data buffer
            void* mapped_data = renderer->MapBufferAs<void>(data);
            std::memcpy(mapped_data, octree.leafData.data(), octree.leafData.size() * data_size);
        }

        void Update()
        {
            frame++;
            camera.setViewportSize(renderer->surface_width, renderer->surface_height);

            // Check if R key was pressed and regenerate the world
            if (Input::WasKeyPressed(Key::R))
            {
                RegenerateWorld();
            }
        }

        void InitializeTasks(daxa::TaskGraph &task_graph, daxa::TaskImage *task_render_image, daxa::ImageId *render_image)
        {
            task_graph.use_persistent_image(*task_render_image);
            task_graph.use_persistent_buffer(task_nodes);
            task_graph.use_persistent_buffer(task_data);

            // create task to compute render
            renderer->AddTask(InlineTask("render")
                              .AddAttachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, *task_render_image)
                              .AddAllAttachments(daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE_CONCURRENT, {task_nodes, task_data})
                              .SetTask([this, render_image](daxa::TaskInterface ti)
            {
                auto push = ComputePush
                {
                    .voxelMap = {1, renderer->GetDeviceAddress(ti, task_nodes), renderer->GetDeviceAddress(ti, task_data)},
                    .screen = render_image->default_view(),
                    .frame_dim = {renderer->surface_width, renderer->surface_height},
                    .time = time,
                    .frame_number = frame,
                    .cameraPos = (camera.getTransform()->GetPosition()).toDaxa(),
                    .invViewProjMat = (camera.getProjectionMatrix() * camera.getViewMatrix()).inverse().toDaxa(),
                };

                ti.recorder.set_pipeline(*render_compute);
                ti.recorder.push_constant(push);
                ti.recorder.dispatch({(renderer->surface_width + 15) / 16, (renderer->surface_height + 15) / 16});
            }));

            // create task to blit render image to swapchain
            renderer->AddTask(renderer->CreateSwapchainBlitTask(*task_render_image));
        }

        void Resize(u32 width, u32 height)
        {
        }
};