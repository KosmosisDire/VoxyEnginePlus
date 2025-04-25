#include "core/core.hpp"
#include "core/menu-page.hpp"
#include "core/settings-page.hpp"
// clang-format off

static bool init = false;

void DrawFrameTime(float dt)
{
    static float dtAvg = 0.0f;
    dtAvg = dtAvg * 0.99f + dt * 0.01f;
    UITEXT(Text(fmt::format("Frame Time: {:.2f}ms", dtAvg * 1000.0f)));
}

void DrawCameraPos(const CameraData& camera)
{
    UITEXT(Text(fmt::format("Camera Pos: {:.2f}, {:.2f}, {:.2f}", camera.position.x, camera.position.y, camera.position.z)));
}


void InitPages(UIData& data)
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

void DrawRootUI(UIData& data, UIInputs input)
{
    if (!init)
    {
        InitPages(data);
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
            DrawFrameTime(input.deltaTime);
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