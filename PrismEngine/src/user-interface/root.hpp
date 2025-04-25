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


void init_pages(UIData& data)
{
    data.pages["Menu"] =
    {
        .isActive = false,
        .render_page = draw_menu_page
    };

    data.pages["Settings"] =
    {
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
       .center_content())
    {
        // display frame time in top corner
        UI(Element("FloatingCorner")
           .floating_attach_point(AttachPointType::RightTop)
           .floating_offset(-10, 10)
           .direction(FlowDirection::TopToBottom))
        {
            draw_frame_time(input.deltaTime);
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