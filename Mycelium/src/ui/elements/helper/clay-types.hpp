#pragma once

#include <clay.h>
#include <imgui_impl_glfw.h>
#include <string>
#include <engine/data/Color.hpp>

// Modern enum wrappers for Clay types
enum class FlowDirection
{
    LeftToRight = CLAY_LEFT_TO_RIGHT,
    TopToBottom = CLAY_TOP_TO_BOTTOM
};

enum class AlignmentX
{
    Left = CLAY_ALIGN_X_LEFT,
    Right = CLAY_ALIGN_X_RIGHT,
    Center = CLAY_ALIGN_X_CENTER
};

enum class AlignmentY
{
    Top = CLAY_ALIGN_Y_TOP,
    Bottom = CLAY_ALIGN_Y_BOTTOM,
    Center = CLAY_ALIGN_Y_CENTER
};

enum class SizingType
{
    Fit = CLAY__SIZING_TYPE_FIT,
    Grow = CLAY__SIZING_TYPE_GROW,
    Percent = CLAY__SIZING_TYPE_PERCENT,
    Fixed = CLAY__SIZING_TYPE_FIXED
};

enum class PointerEventMode
{
    Hit = CLAY_POINTER_CAPTURE_MODE_CAPTURE,
    Passthrough = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH
};

enum class AttachPointType
{
    LeftTop = CLAY_ATTACH_POINT_LEFT_TOP,
    LeftCenter = CLAY_ATTACH_POINT_LEFT_CENTER,
    LeftBottom = CLAY_ATTACH_POINT_LEFT_BOTTOM,
    CenterTop = CLAY_ATTACH_POINT_CENTER_TOP,
    Center = CLAY_ATTACH_POINT_CENTER_CENTER,
    CenterBottom = CLAY_ATTACH_POINT_CENTER_BOTTOM,
    RightTop = CLAY_ATTACH_POINT_RIGHT_TOP,
    RightCenter = CLAY_ATTACH_POINT_RIGHT_CENTER,
    RightBottom = CLAY_ATTACH_POINT_RIGHT_BOTTOM
};

enum class TextWrapMode
{
    None = CLAY_TEXT_WRAP_NONE,
    Words = CLAY_TEXT_WRAP_WORDS,
    Newlines = CLAY_TEXT_WRAP_NEWLINES
};

struct UIInputs
{
    int screenWidth;
    int screenHeight;
    float pointerX;
    float pointerY;
    bool pointerDown;
    float scrollDeltaX;
    float scrollDeltaY;
    float deltaTime;
};

enum ThemeColor
{
    TextColor,
    TextDisabled,
    WindowBg,
    ChildBg,
    PopupBg,
    Border,
    BorderShadow,
    FrameBg,
    FrameBgHovered,
    FrameBgActive,
    TitleBg,
    TitleBgActive,
    TitleBgCollapsed,
    MenuBarBg,
    ScrollbarBg,
    ScrollbarGrab,
    ScrollbarGrabHovered,
    ScrollbarGrabActive,
    CheckMark,
    SliderGrab,
    SliderGrabActive,
    Button,
    ButtonHovered,
    ButtonActive,
    Header,
    HeaderHovered,
    HeaderActive,
    Separator,
    SeparatorHovered,
    SeparatorActive,
    ResizeGrip,
    ResizeGripHovered,
    ResizeGripActive,
    TabHovered,
    Tab,
    TabSelected,
    TabSelectedOverline,
    TabDimmed,
    TabDimmedSelected,
    TabDimmedSelectedOverline,
    DockingPreview,
    DockingEmptyBg,
    PlotLines,
    PlotLinesHovered,
    PlotHistogram,
    PlotHistogramHovered,
    TableHeaderBg,
    TableBorderStrong,
    TableBorderLight,
    TableRowBg,
    TableRowBgAlt,
    TextLink,
    TextSelectedBg,
    DragDropTarget,
    NavCursor,
    NavWindowingHighlight,
    NavWindowingDimBg,
    ModalWindowDimBg,
    COUNT,
};

// Convenience type aliases
using Clay_HoverCallback = void (*)(Clay_ElementId elementId, Clay_PointerData pointer, intptr_t userData);
using ElementId = Clay_ElementId;
using PointerData = Clay_PointerData;
using BoundingBox = Clay_BoundingBox;

// Helper conversion functions
inline Clay_LayoutDirection to_clay(FlowDirection dir)
{
    return static_cast<Clay_LayoutDirection>(dir);
}

inline Clay_LayoutAlignmentX to_clay(AlignmentX align)
{
    return static_cast<Clay_LayoutAlignmentX>(align);
}

inline Clay_LayoutAlignmentY to_clay(AlignmentY align)
{
    return static_cast<Clay_LayoutAlignmentY>(align);
}

inline Clay__SizingType to_clay(SizingType type)
{
    return static_cast<Clay__SizingType>(type);
}

inline Clay_PointerCaptureMode to_clay(PointerEventMode mode)
{
    return static_cast<Clay_PointerCaptureMode>(mode);
}

inline Clay_FloatingAttachPointType to_clay(AttachPointType point)
{
    return static_cast<Clay_FloatingAttachPointType>(point);
}

inline Clay_TextElementConfigWrapMode to_clay(TextWrapMode mode)
{
    return static_cast<Clay_TextElementConfigWrapMode>(mode);
}

inline Clay_Color to_clay(const Color &color)
{
    return {color.r, color.g, color.b, color.a};
}

#define ImGuiColor(ThemeColor) Color(ImGui::GetStyle().Colors[ThemeColor].x * 255, ImGui::GetStyle().Colors[ThemeColor].y * 255, ImGui::GetStyle().Colors[ThemeColor].z * 255, ImGui::GetStyle().Colors[ThemeColor].w * 255)


// Default configurations
constexpr Clay_LayoutConfig DEFAULT_LAYOUT = {.sizing = {.width = {.size = {.minMax = {0, 3.40282346638528859812e+38F}}, .type = CLAY__SIZING_TYPE_FIT}, .height = {.size = {.minMax = {0, 3.40282346638528859812e+38F}}, .type = CLAY__SIZING_TYPE_FIT}}};

constexpr Clay_RectangleElementConfig DEFAULT_RECTANGLE =
{
    .color = {0, 0, 0, 0},
    .cornerRadius = {0, 0, 0, 0}
};

constexpr Clay_TextElementConfig DEFAULT_TEXT =
{
    .textColor = {255, 255, 255, 255},
    .fontId = 0,
    .fontSize = 16,
    .letterSpacing = 0,
    .lineHeight = 0,
    .wrapMode = CLAY_TEXT_WRAP_WORDS
};

constexpr Clay_ScrollElementConfig DEFAULT_SCROLL =
{
    .horizontal = false,
    .vertical = false
};

constexpr Clay_BorderElementConfig DEFAULT_BORDER =
{
    .left = {0, {0, 0, 0, 0}},
    .right = {0, {0, 0, 0, 0}},
    .top = {0, {0, 0, 0, 0}},
    .bottom = {0, {0, 0, 0, 0}},
    .betweenChildren = {0, {0, 0, 0, 0}},
    .cornerRadius = {0, 0, 0, 0}
};

constexpr Clay_FloatingElementConfig DEFAULT_FLOATING =
{
    .offset = {0, 0},
    .expand = {0, 0},
    .zIndex = 0,
    .parentId = 0,
    .attachment = {Clay_FloatingAttachPointType::CLAY_ATTACH_POINT_LEFT_TOP, Clay_FloatingAttachPointType::CLAY_ATTACH_POINT_LEFT_TOP},
    .pointerCaptureMode = Clay_PointerCaptureMode::CLAY_POINTER_CAPTURE_MODE_CAPTURE
};

struct ClayConfigs
{
    Clay_ElementId clayId;
    Clay_LayoutConfig layout = DEFAULT_LAYOUT;
    Clay_RectangleElementConfig rectangle = DEFAULT_RECTANGLE;
    Clay_TextElementConfig text = DEFAULT_TEXT;
    Clay_ScrollElementConfig scroll = DEFAULT_SCROLL;
    Clay_BorderElementConfig border = DEFAULT_BORDER;
    Clay_FloatingElementConfig floating = DEFAULT_FLOATING;

    uint32_t activeConfigs = 0;
};

struct ComputedProps
{
    float width = 0;
    float height = 0;
    float x = 0;
    float y = 0;
    uint32_t parentId;
    ComputedProps *parent = nullptr;
    uint32_t elementDepth = 0;
};

// Element properties container
struct ElementProps
{
    std::string id;
    std::string textContent;

    ClayConfigs clay;
    ComputedProps computed;
};