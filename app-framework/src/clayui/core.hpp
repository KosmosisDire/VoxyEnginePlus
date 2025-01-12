#pragma once

#include "../fonts.hpp"
#include "elements/element.hpp"
#include "elements/helper/clay-imgui-renderer.hpp"
#include "elements/text.hpp"

class ClayUI
{
  private:
    static bool initialized;

    static void HandleClayErrors(Clay_ErrorData errorData)
    {
        printf("%s", errorData.errorText.chars);
    }

    static void Initialize(UIInputs input)
    {
        uint64_t totalMemorySize = Clay_MinMemorySize();
        Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, (char *)malloc(totalMemorySize));
        Clay_Initialize(clayMemory, Clay_Dimensions{(float)input.screenWidth, (float)input.screenHeight}, Clay_ErrorHandler{HandleClayErrors});
        Clay_SetMeasureTextFunction(Imgui_MeasureText);
        Clay_SetDebugModeEnabled(true);

        Clay_SetFont(FontManager::GetFont("Roboto"));

        ClayState::lastPointerX = input.pointerX;
        ClayState::lastPointerY = input.pointerY;

        SetInputs(input);

        initialized = true;
    }

    static void SetInputs(UIInputs input)
    {
        ClayState::inputs = input;
        ClayState::pointerDeltaX = input.pointerX - ClayState::lastPointerX;
        ClayState::pointerDeltaY = input.pointerY - ClayState::lastPointerY;
        ClayState::lastPointerX = input.pointerX;
        ClayState::lastPointerY = input.pointerY;

        Clay_SetLayoutDimensions(Clay_Dimensions{(float)input.screenWidth, (float)input.screenHeight});
        Clay_SetPointerState(Clay_Vector2{input.pointerX, input.pointerY}, input.pointerDown);
        Clay_UpdateScrollContainers(true, Clay_Vector2{input.scrollDeltaX, input.scrollDeltaY * 10}, input.deltaTime);
    }

    static void EndFrame()
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
        SetInputs(input);

        Element::ClearStack();
        Clay_BeginLayout();
    }

  public:
    template <typename T>
    static void Update(T &state, UIInputs input, std::function<void(T &, UIInputs)> build_ui)
    {
        // frameStart = std::chrono::high_resolution_clock::now();
        BeginFrame(input);
        build_ui(state, input);
        EndFrame();
        // uiFrameTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - frameStart).count();
    }
};

bool ClayUI::initialized = false;