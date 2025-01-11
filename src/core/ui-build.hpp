#pragma once

// clang-format off

#include <fmt/core.h>
#include <string>
#include <clayui/core.hpp>
#include <daxa/daxa.hpp>
using namespace daxa::types;
using namespace std;

// Settings structures
struct GraphicsSettings {
    int resolutionIndex = 0;
    bool fullscreen = false;
    bool vsync = true;
    int qualityPreset = 2;
};

struct AudioSettings {
    float masterVolume = 1.0f;
    float musicVolume = 0.8f;
    float effectsVolume = 0.8f;
};

struct ControlsSettings {
    float mouseSensitivity = 1.0f;
    bool invertMouseY = false;
    bool invertMouseX = false;
};

struct Settings {
    GraphicsSettings graphics;
    AudioSettings audio;
    ControlsSettings controls;
};

struct UIState {
    float time;
    daxa_f32vec3 backgroundColor;
    bool menuOpen;
    int selectedPage = 0;
    Settings settings;
    bool dropdownOpen = false;
    int activeDropdownId = -1;
};

// UI Components
void SliderControl(const char* label, float& value, float min, float max)
{
    UI(Element(label)
    .width(SizingType::Grow)
    .height(40)
    .padding(5, 5)
    .direction(FlowDirection::LeftToRight)
    .gap(10))
    {
        // Label
        UITEXT(Text(label)
        .color(ThemeColor::TextColor)
        .fontSize(16));

        const float sliderSize = 20;

        // Slider track
        UI(Element("SliderTrack" + string(label))
        .grow()
        .height(sliderSize)
        .color(ThemeColor::FrameBg)
        .cornerRadius(sliderSize/2.0)
        .OnDrag([min, max, &value, sliderSize](Element& el, ComputedProps props, UIInputs input)
        {
            el.CaptureDrag();
            float parentWidth = props.width;
            float elX = input.pointerX - props.x + sliderSize / 2.0;

            //clamp elX to inner range
            // elX = std::clamp(elX, sliderSize, parentWidth);

            float normalizedValue = elX / (parentWidth);
            value = std::clamp(normalizedValue * (max - min) + min, min, max);
        }))
        {
            const float normalizedValue = (value - min) / (max - min);
            const float handleOffset = normalizedValue * (Computed().width);
            
            // Fill bar
            UI(Element("SliderFill" + string(label))
            .width(handleOffset)
            .height(sliderSize)
            .color(ThemeColor::ButtonActive)
            .cornerRadius(sliderSize / 2.0)){}

            // Handle
            UI(Element("SliderHandle" + string(label))
            .width(sliderSize)
            .height(sliderSize)
            .floatingOffsetX(handleOffset - sliderSize)
            .floatingPointerMode(PointerEventMode::Passthrough)
            .color(ThemeColor::SliderGrab, ThemeColor::SliderGrabActive, ThemeColor::SliderGrabActive)
            .cornerRadius(sliderSize / 2.0));
        }

        UITEXT(Text(fmt::format("{:.2f}", value))
            .color(ThemeColor::TextColor)
            .fontSize(16));
    }
}

void ToggleControl(const char* label, bool& value) {
    UI(Element(label)
    .width(SizingType::Grow)
    .height(40)
    .padding(5, 5)
    .direction(FlowDirection::LeftToRight)
    .gap(10))
    {
        // Label
        UITEXT(Text(label)
        .color(ThemeColor::TextColor)
        .fontSize(16));

        // Toggle switch
        UI(Element("Toggle" + string(label))
        .width(50)
        .height(24)
        .color(value ? ThemeColor::ButtonActive : ThemeColor::FrameBg)
        .cornerRadius(12)
        .OnClick<bool>(value, [](Element &el, ComputedProps props, UIInputs input, bool &val)
        {
            val = !val;
        }))
        {
            // Toggle handle
            UI(Element("Handle" + string(label))
            .width(20)
            .height(20)
            .floatingOffset(value ? 27 : 2, 2)
            .floatingZIndex(-100)
            .floatingPointerMode(PointerEventMode::Passthrough)
            .color(ThemeColor::TextColor)
            .cornerRadius(10)){}
        }
    }
}

void DropdownControl(const char* label, int& value, const char** options, int optionCount, UIState& state, const std::string& id) {
    const bool isActive = state.dropdownOpen && state.activeDropdownId == ClayState::HashId(id).id;
    
    UI(Element(id)
    .width(SizingType::Grow)
    .height(40)
    .padding(5, 5)
    .direction(FlowDirection::LeftToRight)
    .centerContentY()
    .gap(10))
    {
        // Label
        UITEXT(Text(label)
            .color(ThemeColor::TextColor)
            .fontSize(16));

        // Current selection
        UI(Element("DropdownHeader" + string(label))
        .grow()
        .height(30)
        .centerContentY()
        .padding(12, 0)
        .color(isActive ? ThemeColor::ButtonActive : ThemeColor::Button)
        .cornerRadius(5)
        .OnClick([&state, id](Element& el, ComputedProps props, UIInputs input)
        {
            if (state.dropdownOpen && state.activeDropdownId == ElementIdFromString(id)) {
                state.dropdownOpen = false;
                state.activeDropdownId = -1;
            } else {
                state.dropdownOpen = true;
                state.activeDropdownId = ElementIdFromString(id);
            }
        }))
        {
            UITEXT(Text(options[value])
            .color(ThemeColor::TextColor)
            .fontSize(16));

            if (isActive) {
                UI(Element("DropdownList" + string(label))
                .width(SizingType::Grow)
                .floatingOffsetY(35)
                .color(ThemeColor::WindowBg)
                .direction(FlowDirection::TopToBottom)
                .cornerRadius(5)
                .floatingZIndex(10000000000)
                .border(1, {1.0f, 1.0f, 1.0f, 0.1f}))
                {
                    for (int i = 0; i < optionCount; i++) {
                        UI(Element(options[i])
                        .width(SizingType::Grow)
                        .height(30)
                        .padding(12, 5)
                        .color(IsHovered() ? ThemeColor::ButtonActive : ((i == value) ? ThemeColor::ButtonHovered : ThemeColor::Button))
                        .OnClick([i, &value, &state](Element &el, ComputedProps props, UIInputs input)
                        {
                            value = i;
                            state.dropdownOpen = false;
                            state.activeDropdownId = -1;
                        }))
                        {
                            UITEXT(Text(options[i])
                            .color(ThemeColor::TextColor)
                            .fontSize(16));
                        }
                    }
                }
            }
        }
    }
}

const char* settingTitles[3] = {
    "Graphics",
    "Audio",
    "Controls",
};

const char* qualityOptions[4] = {
    "Low",
    "Medium",
    "High",
    "Ultra"
};

const char* resolutionOptions[3] = {
    "1920x1080",
    "2560x1440",
    "3840x2160"
};

void graphicsPage(UIState& state, UIInputs input) {
    UI(Element("GraphicsSettings")
    .grow()
    .padding(10, 10)
    .gap(15)
    .direction(FlowDirection::TopToBottom))
    {
        DropdownControl("Resolution", state.settings.graphics.resolutionIndex, 
                       resolutionOptions, 3, state, "resolution_dropdown");
        
        ToggleControl("Fullscreen", state.settings.graphics.fullscreen);
        
        ToggleControl("VSync", state.settings.graphics.vsync);
        
        DropdownControl("Quality Preset", state.settings.graphics.qualityPreset,
                       qualityOptions, 4, state, "quality_dropdown");
    }
}

void audioPage(UIState& state, UIInputs input) {
    UI(Element("AudioSettings")
    .grow()
    .padding(10, 10)
    .gap(15)
    .direction(FlowDirection::TopToBottom))
    {
        SliderControl("Master Volume", state.settings.audio.masterVolume, 
                     0.0f, 1.0f);
        
        SliderControl("Music Volume", state.settings.audio.musicVolume,
                     0.0f, 1.0f);
        
        SliderControl("Effects Volume", state.settings.audio.effectsVolume,
                     0.0f, 1.0f);
    }
}

void controlsPage(UIState& state, UIInputs input) {
    UI(Element("ControlsSettings")
    .grow()
    .padding(10, 10)
    .gap(15)
    .direction(FlowDirection::TopToBottom))
    {
        SliderControl("Mouse Sensitivity", state.settings.controls.mouseSensitivity,
                     0.1f, 2.0f);
        
        ToggleControl("Invert Mouse Y", state.settings.controls.invertMouseY);
        
        ToggleControl("Invert Mouse X", state.settings.controls.invertMouseX);
    }
}

void build_ui(UIState& state, UIInputs input) {
    if (!state.menuOpen) {
        return;
    }
    
    UI(Element("Root").grow().align(AlignmentX::Center, AlignmentY::Center))
    {
        UI(Element("MainContainer")
        .size(SizingType::Percent, 50, 50)
        .color(ThemeColor::WindowBg)
        .padding(10, 10)
        .gap(10)
        .cornerRadius(10)
        .direction(FlowDirection::TopToBottom))
        {
            UITEXT(Text("Settings")
            .color(ThemeColor::TextColor)
            .fontSize(24));

            UI(Element("SettingsContainer")
            .direction(FlowDirection::LeftToRight)
            .grow())
            {
                // Left container with setting categories
                UI(Element("LeftContainer")
                .width(200)
                .grow()
                .padding(5, 5)
                .gap(10)
                .direction(FlowDirection::TopToBottom))
                {
                    for (int i = 0; i < 3; i++) {
                        auto title = settingTitles[i];
                        UI(Element(title)
                        .width(SizingType::Grow)
                        .height(50)
                        .color(i == state.selectedPage ? ThemeColor::ButtonActive : ThemeColor::FrameBg)
                        .cornerRadius(10)
                        .padding(10, 10)
                        .align(AlignmentX::Left, AlignmentY::Center)
                        .OnClick([i, &state](Element& el, ComputedProps props, UIInputs input)
                        {
                            state.selectedPage = i;
                            state.dropdownOpen = false;
                            state.activeDropdownId = -1;
                        }))
                        {
                            UITEXT(Text(title)
                                .color(ThemeColor::TextColor)
                                .fontSize(20));
                        }
                    }
                }

                // Right container with settings content
                UI(Element("RightContainer")
                .grow()
                .color(ThemeColor::FrameBg)
                .cornerRadius(10))
                {
                    switch (state.selectedPage) {
                        case 0: graphicsPage(state, input); break;
                        case 1: audioPage(state, input); break;
                        case 2: controlsPage(state, input); break;
                    }
                }
            }

            // Close button (top-right corner)
            UI(Element("CloseButton")
            .width(20)
            .height(20)
            .color({0.81f, 0.26f, 0.35f, 0.5f}, {0.81f, 0.26f, 0.35f, 0.7f}, {0.81f, 0.26f, 0.35f, 1.0f})
            .cornerRadius(3)
            .border(1, {1.0f, 1.0f, 1.0f, 0.5f})
            .floatingOffset(-10, 10)
            .floatingAttachPointParent(AttachPointType::RightTop, AttachPointType::RightTop)
            .floatingAttachPointSelf(AttachPointType::RightTop, AttachPointType::RightTop)
            .centerContent()
            .OnClick([&state](Element& el, ComputedProps props, UIInputs input)
            {
                state.menuOpen = false;
            }))
            {
                UITEXT(Text("×")
                    .color(ThemeColor::TextColor)
                    .fontSize(16));
            }
        }
    }
}


















// Normal clay syntax:

// Clay_Sizing layoutExpand =
// {
//     .width = CLAY_SIZING_GROW,
//     .height = CLAY_SIZING_GROW,
// };

// const Clay_String settingTitles[10] =
// {
//     CLAY_STRING("Setting 0"),
//     CLAY_STRING("Setting 1"),
//     CLAY_STRING("Setting 2"),
//     CLAY_STRING("Setting 3"),
//     CLAY_STRING("Setting 4"),
//     CLAY_STRING("Setting 5"),
//     CLAY_STRING("Setting 6"),
//     CLAY_STRING("Setting 7"),
//     CLAY_STRING("Setting 8"),
//     CLAY_STRING("Setting 9"),
// };

// void close_button(Clay_ElementId id, Clay_PointerData pointer, intptr_t userData)
// {
//     UIState *state = (UIState *)userData;
//     if (pointer.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
//     {
//         state->menuOpen = false;
//     }
// }

// void build_ui(UIState state, UIInputs input)
// {
//     if (!state.menuOpen)
//     {
//         return;
//     }

//     CLAY(
//         CLAY_ID("InnerContainer"),
//         CLAY_RECTANGLE(),
//         CLAY_LAYOUT({
//             .sizing = {
//                 .width = CLAY_SIZING_PERCENT(0.5),
//                 .height = CLAY_SIZING_PERCENT(0.5),
//             },
//             .childGap = 10,
//             .layoutDirection = Clay_LayoutDirection::CLAY_LEFT_TO_RIGHT,
//         }))
//     {
//         // close button
//         CLAY(
//             CLAY_ID("CloseButton"),
//             CLAY_RECTANGLE({
//                 .color = ImGuiColor(ColorType::Button),
//                 .cornerRadius = {10, 10, 10, 10},
//             }),
//             CLAY_LAYOUT({
//                 .sizing = {
//                     .width = CLAY_SIZING_FIXED(50),
//                     .height = CLAY_SIZING_FIXED(50),
//                 },
//                 .childAlignment = {Clay_LayoutAlignmentX::CLAY_ALIGN_X_CENTER, Clay_LayoutAlignmentY::CLAY_ALIGN_Y_CENTER},
//             }),
//             Clay_OnHover(close_button, (intptr_t)&state)
//         )
//         {
//             CLAY_TEXT(
//                 CLAY_STRING("X"),
//                 CLAY_TEXT_CONFIG({
//                     .textColor = ImGuiColor(ColorType::Text),
//                     .fontSize = 20,
//                 }));
//         }
//         CLAY(
//             CLAY_ID("LeftContainer"),
//             CLAY_RECTANGLE({
//                 .color = ImGuiColor(ColorType::FrameBg),
//                 .cornerRadius = {20, 20, 20, 20},
//             }),
//             CLAY_LAYOUT({
//                 .sizing = layoutExpand,
//                 .padding = {10, 10},
//                 .childGap = 10,
//                 .layoutDirection = Clay_LayoutDirection::CLAY_TOP_TO_BOTTOM,
//             }))
//         {
//             // create 10 settings boxes with titles
//             for (int i = 0; i < 10; i++)
//             {
//                 Clay_String title = settingTitles[i];

//                 CLAY(
//                     CLAY_RECTANGLE({
//                         .color = ImGuiColor(ColorType::FrameBg),
//                         .cornerRadius = {10, 10, 10, 10},
//                     }),
//                     CLAY_LAYOUT({
//                         .sizing = layoutExpand,
//                         .padding = {10, 10},
//                         .childGap = 10,
//                         .childAlignment = {Clay_LayoutAlignmentX::CLAY_ALIGN_X_LEFT, Clay_LayoutAlignmentY::CLAY_ALIGN_Y_CENTER},
//                         .layoutDirection = Clay_LayoutDirection::CLAY_TOP_TO_BOTTOM,
//                     })
//                 )
//                 {
//                     CLAY_TEXT(title, CLAY_TEXT_CONFIG({
//                         .textColor = ImGuiColor(ColorType::Text),
//                         .fontSize = 20,
//                     }));
//                 }
//             }
//         }

//         CLAY(
//             CLAY_ID("RightContainer"),
//             CLAY_RECTANGLE({
//                 .color = ImGuiColor(ColorType::FrameBg),
//                 .cornerRadius = {20, 20, 20, 20},
//             }),
//             CLAY_LAYOUT({
//                 .sizing = layoutExpand,
//                 .childGap = 10,
//                 .layoutDirection = Clay_LayoutDirection::CLAY_TOP_TO_BOTTOM,
//             }))
//         {
//         }
//     }
// }