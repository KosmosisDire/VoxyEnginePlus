#pragma once

#include "clay-text.hpp"
#include "element.hpp"

class ClayUI
{
  private:
    static void ClayUI::EndFrame()
    {
        Clay_RenderCommandArray renderCommands = Clay_EndLayout();
        ClayState::FillComputedProperties(renderCommands);
        clay_imgui_render(renderCommands);
    }

    static void BeginFrame(UIInputs input)
    {
        if (!initialized)
        {
            Initialize(input);
        }

        ClayState::FreeStrings();
        ClayState::SetInputs(input);

        Element::ClearStack();
        Clay_BeginLayout();
    }

  public:
    template <typename T>
    static void ClayUI::Update(T &state, UIInputs input, std::function<void(T &, UIInputs)> build_ui)
    {
        // frameStart = std::chrono::high_resolution_clock::now();
        BeginFrame(input);
        build_ui(state, input);
        Render();
        // uiFrameTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - frameStart).count();
        // printf("UI frame time: %f\n", uiFrameTime);
    }
};
