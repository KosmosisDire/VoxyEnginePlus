#pragma once

#include <engine/apis/Fonts.hpp>
#include <functional>
#include "elements/element.hpp"
#include "elements/helper/clay-imgui-renderer.hpp"
#include "elements/text.hpp"

class ClayUI
{
    private:
        static bool initialized;
        static bool debugMode;

        static void handle_clay_errors(Clay_ErrorData errorData)
        {
            printf("%s", errorData.errorText.chars);
        }

        static void initialize(UIInputs input)
        {
            uint64_t totalMemorySize = Clay_MinMemorySize();
            Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, (char*)malloc(totalMemorySize));
            Clay_Initialize(clayMemory, Clay_Dimensions{(float)input.screenWidth, (float)input.screenHeight}, Clay_ErrorHandler{handle_clay_errors});
            Clay_SetMeasureTextFunction(imgui_measure_text);

            clay_set_font(FontManager::get_font("Roboto"));

            ClayState::lastPointerX = input.pointerX;
            ClayState::lastPointerY = input.pointerY;

            set_inputs(input);

            initialized = true;
        }

        static void set_inputs(UIInputs input)
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

        static void end_frame()
        {
            Clay_RenderCommandArray renderCommands = Clay_EndLayout();
            ClayState::fill_computed_properties(renderCommands);
            clay_imgui_render(renderCommands);
        }

        static void begin_frame(UIInputs input)
        {
            if (!initialized)
            {
                initialize(input);
            }

            ClayState::free_strings();
            set_inputs(input);

            Element::clear_stack();
            Clay_BeginLayout();
        }

    public:
        template <typename T>
        static void update(T &state, UIInputs input, std::function<void(T &, UIInputs)> build_ui)
        {
            // frameStart = std::chrono::high_resolution_clock::now();
            begin_frame(input);
            build_ui(state, input);
            end_frame();
            // uiFrameTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - frameStart).count();
        }

        static void debug_mode(bool enabled)
        {
            debugMode = enabled;
            Clay_SetDebugModeEnabled(enabled);
        }

        static bool get_debug_mode()
        {
            return debugMode;
        }
};

bool ClayUI::initialized = false;
bool ClayUI::debugMode = false;