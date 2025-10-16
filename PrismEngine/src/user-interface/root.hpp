#include "core/core.hpp"
#include "core/menu-page.hpp"
#include "core/settings-page.hpp"
// clang-format off

static bool init = false;

void draw_frame_time(float dt)
{
    static float dtAvg = 0.0f;
    dtAvg = dtAvg * 0.99f + dt * 0.01f;
    UITEXT(Text(fmt::format("Frame Time: {:.2f}ms", dtAvg * 1000.0f)));
}

void draw_camera_pos(const CameraData& camera)
{
    UITEXT(Text(fmt::format("Camera Pos: {:.2f}, {:.2f}, {:.2f}", camera.position.x, camera.position.y, camera.position.z)));
}

void draw_gpu_timings(const GPUTimings& timings)
{
    // Smooth the timings for more stable display
    static GPUTimings smoothed = {};
    const float alpha = 0.15f;  // Smoothing factor

    smoothed.totalFrame = smoothed.totalFrame * (1.0f - alpha) + timings.totalFrame * alpha;
    smoothed.gbufferTrace = smoothed.gbufferTrace * (1.0f - alpha) + timings.gbufferTrace * alpha;
    smoothed.gtaoPass = smoothed.gtaoPass * (1.0f - alpha) + timings.gtaoPass * alpha;
    smoothed.aoApply = smoothed.aoApply * (1.0f - alpha) + timings.aoApply * alpha;
    smoothed.composite = smoothed.composite * (1.0f - alpha) + timings.composite * alpha;
    smoothed.terrainGen = smoothed.terrainGen * (1.0f - alpha) + timings.terrainGen * alpha;

    UITEXT(Text("GPU Timings:"));
    if (smoothed.totalFrame > 0.01f)
    {
        UITEXT(Text(fmt::format("  Total Frame: {:.2f}ms", smoothed.totalFrame)));
        UITEXT(Text(fmt::format("  GBuffer Trace: {:.2f}ms", smoothed.gbufferTrace)));
        UITEXT(Text(fmt::format("  GTAO Pass: {:.2f}ms", smoothed.gtaoPass)));
        UITEXT(Text(fmt::format("  AO Apply: {:.2f}ms", smoothed.aoApply)));
        UITEXT(Text(fmt::format("  Composite: {:.2f}ms", smoothed.composite)));
        if (smoothed.terrainGen > 0.01f)
        {
            UITEXT(Text(fmt::format("  Terrain Gen: {:.2f}ms", smoothed.terrainGen)));
        }
    }
    else
    {
        UITEXT(Text("  (Warming up...)"));
    }
}


void init_pages(UIData& data)
{
    data.pages["Menu"] = {
        .isActive = false,
        .render_page = draw_menu_page
    };

    data.pages["Settings"] = {
        .isActive = true,
        .render_page = draw_settings_page
    };

    init = true;
}

void draw_root_ui(UIData& data, UIInputs input)
{
    if (!init)
    {
        init_pages(data);
    }

    UI(Element("Root")
    .grow()
    .centerContent())
    {
        // display frame time in top corner
        UI(Element("FloatingCorner")
        .floatingAttachPoint(AttachPointType::RightTop)
        .floatingOffset(-10, 10)
        .direction(FlowDirection::TopToBottom))
        {
            draw_frame_time(input.deltaTime);
            draw_camera_pos(data.renderData->camera);
            draw_gpu_timings(data.gpuTimings);
        }

        if (!data.mouseIsActive) continue; // is this how this should be used?

        for (const auto& pageEntry : data.pages)
        {
            auto pageName = pageEntry.first;
            auto page = pageEntry.second;

            if (page.isActive)
            {
                page.render_page(data, input);
            }
        }

    }
}