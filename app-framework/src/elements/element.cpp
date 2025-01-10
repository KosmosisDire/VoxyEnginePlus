// clay-elements.cpp
#include "clay-elements.hpp"
#include "clay-ui.hpp"
#include <clay.h>
#include <stdexcept>

Element::Element(std::string id)
{
    parent = ClayUI::PeekStack();
    if (parent != nullptr)
    {
        props.computed.parentId = parent->props.clay.clayId.id;
    }

    props.id = std::move(id);
    props.clay.clayId = ClayUI::HashId(props.id);

    ClayUI::PushStack(*this);
}

Element &Element::size(SizingType type, float width, float height)
{
    switch (type)
    {
    case SizingType::Fixed:
        props.clay.layout.sizing.width = Clay_SizingAxis{{{width, width}}, CLAY__SIZING_TYPE_FIXED};
        props.clay.layout.sizing.height = Clay_SizingAxis{{{height, height}}, CLAY__SIZING_TYPE_FIXED};
        break;
    case SizingType::Percent:
        props.clay.layout.sizing.width = Clay_SizingAxis{{{width / 100.0f}}, CLAY__SIZING_TYPE_PERCENT};
        props.clay.layout.sizing.height = Clay_SizingAxis{{{height / 100.0f}}, CLAY__SIZING_TYPE_PERCENT};
        break;
    case SizingType::Grow:
        props.clay.layout.sizing.width = Clay_SizingAxis{{{0, 0}}, CLAY__SIZING_TYPE_GROW};
        props.clay.layout.sizing.height = Clay_SizingAxis{{{0, 0}}, CLAY__SIZING_TYPE_GROW};
        break;
    case SizingType::Fit:
        props.clay.layout.sizing.width = Clay_SizingAxis{{{0, 0}}, CLAY__SIZING_TYPE_FIT};
        props.clay.layout.sizing.height = Clay_SizingAxis{{{0, 0}}, CLAY__SIZING_TYPE_FIT};
        break;
    }
    return *this;
}

Element &Element::size(float width, float height)
{
    return size(SizingType::Fixed, width, height);
}

Element &Element::width(SizingType type, float width)
{
    switch (type)
    {
    case SizingType::Fixed:
        props.clay.layout.sizing.width = Clay_SizingAxis{{{width, width}}, CLAY__SIZING_TYPE_FIXED};
        break;
    case SizingType::Percent:
        props.clay.layout.sizing.width = Clay_SizingAxis{{{width / 100.0f}}, CLAY__SIZING_TYPE_PERCENT};
        break;
    case SizingType::Grow:
        props.clay.layout.sizing.width = Clay_SizingAxis{{{0, 0}}, CLAY__SIZING_TYPE_GROW};
        break;
    case SizingType::Fit:
        props.clay.layout.sizing.width = Clay_SizingAxis{{{0, 0}}, CLAY__SIZING_TYPE_FIT};
        break;
    }
    return *this;
}

Element &Element::height(SizingType type, float height)
{
    switch (type)
    {
    case SizingType::Fixed:
        props.clay.layout.sizing.height = Clay_SizingAxis{{{height, height}}, CLAY__SIZING_TYPE_FIXED};
        break;
    case SizingType::Percent:
        props.clay.layout.sizing.height = Clay_SizingAxis{{{height / 100.0f}}, CLAY__SIZING_TYPE_PERCENT};
        break;
    case SizingType::Grow:
        props.clay.layout.sizing.height = Clay_SizingAxis{{{0, 0}}, CLAY__SIZING_TYPE_GROW};
        break;
    case SizingType::Fit:
        props.clay.layout.sizing.height = Clay_SizingAxis{{{0, 0}}, CLAY__SIZING_TYPE_FIT};
        break;
    }
    return *this;
}

Element &Element::width(float width)
{
    return this->width(SizingType::Fixed, width);
}

Element &Element::height(float height)
{
    return this->height(SizingType::Fixed, height);
}

Element &Element::direction(FlowDirection dir)
{
    props.clay.layout.layoutDirection = ToClay(dir);
    return *this;
}

Element &Element::gap(float gap)
{
    props.clay.layout.childGap = static_cast<uint16_t>(gap);
    return *this;
}

Element &Element::align(AlignmentX x, AlignmentY y)
{
    props.clay.layout.childAlignment.x = ToClay(x);
    props.clay.layout.childAlignment.y = ToClay(y);
    return *this;
}

Element &Element::centerContent()
{
    props.clay.layout.childAlignment.x = CLAY_ALIGN_X_CENTER;
    props.clay.layout.childAlignment.y = CLAY_ALIGN_Y_CENTER;
    return *this;
}

Element &Element::centerContentX()
{
    props.clay.layout.childAlignment.x = CLAY_ALIGN_X_CENTER;
    return *this;
}

Element &Element::centerContentY()
{
    props.clay.layout.childAlignment.y = CLAY_ALIGN_Y_CENTER;
    return *this;
}

Element &Element::padding(float x, float y)
{
    props.clay.layout.padding.x = static_cast<uint16_t>(x);
    props.clay.layout.padding.y = static_cast<uint16_t>(y);
    return *this;
}

Element &Element::color(const Color &color)
{
    props.clay.rectangle.color = {color.r * 255, color.g * 255, color.b * 255, color.a * 255};
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_RECTANGLE;
    return *this;
}

Element &Element::color(const ThemeColor imguiColor)
{
    Color color = ImGuiColor(imguiColor);
    props.clay.rectangle.color = {color.r, color.g, color.b, color.a};
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_RECTANGLE;
    return *this;
}

Element &Element::color(const Color &normalColor, const Color &hoverColor, const Color &pressColor)
{
    this->color(IsPressed() ? pressColor : (IsHovered() ? hoverColor : normalColor));
    return *this;
}

Element &Element::color(const ThemeColor normalColor, const ThemeColor hoverColor, const ThemeColor pressColor)
{
    this->color(IsPressed() ? pressColor : (IsHovered() ? hoverColor : normalColor));
    return *this;
}

Element &Element::cornerRadius(float radius)
{
    props.clay.rectangle.cornerRadius = {radius, radius, radius, radius};
    props.clay.border.cornerRadius = {radius, radius, radius, radius};
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_RECTANGLE;
    return *this;
}

Element &Element::scroll(bool horizontal, bool vertical)
{
    props.clay.scroll.horizontal = horizontal;
    props.clay.scroll.vertical = vertical;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_SCROLL_CONTAINER;
    return *this;
}

Element &Element::border(float width, const Color &color)
{
    auto border = Clay_Border{(uint32_t)width, {color.r * 255, color.g * 255, color.b * 255, color.a * 255}};
    props.clay.border.bottom = border;
    props.clay.border.top = border;
    props.clay.border.left = border;
    props.clay.border.right = border;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_BORDER_CONTAINER;
    return *this;
}

Element &Element::borderWidth(float width)
{
    props.clay.border.bottom.width = (uint32_t)width;
    props.clay.border.top.width = (uint32_t)width;
    props.clay.border.left.width = (uint32_t)width;
    props.clay.border.right.width = (uint32_t)width;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_BORDER_CONTAINER;
    return *this;
}

Element &Element::borderColor(const Color &color)
{
    auto col = Color{color.r * 255, color.g * 255, color.b * 255, color.a * 255};
    props.clay.border.bottom.color = col;
    props.clay.border.top.color = col;
    props.clay.border.left.color = col;
    props.clay.border.right.color = col;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_BORDER_CONTAINER;
    return *this;
}

Element &Element::gapBorder(float width, const Color &color)
{
    props.clay.border.betweenChildren = {(uint32_t)width, {color.r * 255, color.g * 255, color.b * 255, color.a * 255}};
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_BORDER_CONTAINER;
    return *this;
}

Element &Element::floatingOffset(float offsetX, float offsetY)
{
    props.clay.floating.offset.x = offsetX;
    props.clay.floating.offset.y = offsetY;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingOffsetX(float offsetX)
{
    props.clay.floating.offset.x = offsetX;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingOffsetY(float offsetY)
{
    props.clay.floating.offset.y = offsetY;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingExpand(float expandX, float expandY)
{
    props.clay.floating.expand.width = expandX;
    props.clay.floating.expand.height = expandY;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingExpandX(float expandX)
{
    props.clay.floating.expand.width = expandX;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingExpandY(float expandY)
{
    props.clay.floating.expand.height = expandY;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingZIndex(int zIndex)
{
    props.clay.floating.zIndex = zIndex;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingParent(std::string parentId)
{
    props.clay.floating.parentId = ElementIdFromString(parentId);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingParent(int parentId)
{
    props.clay.floating.parentId = parentId;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingAttachPointSelf(AttachPointType attachX, AttachPointType attachY)
{
    props.clay.floating.attachment.element = ToClay(attachX);
    props.clay.floating.attachment.element = ToClay(attachY);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingAttachPointSelfX(AttachPointType attachX)
{
    props.clay.floating.attachment.element = ToClay(attachX);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingAttachPointSelfY(AttachPointType attachY)
{
    props.clay.floating.attachment.element = ToClay(attachY);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingAttachPointParent(AttachPointType attachX, AttachPointType attachY)
{
    props.clay.floating.attachment.parent = ToClay(attachX);
    props.clay.floating.attachment.parent = ToClay(attachY);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingAttachPointParentX(AttachPointType attachX)
{
    props.clay.floating.attachment.parent = ToClay(attachX);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingAttachPointParentY(AttachPointType attachY)
{
    props.clay.floating.attachment.parent = ToClay(attachY);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floatingPointerMode(PointerEventMode captureMode)
{
    props.clay.floating.pointerCaptureMode = ToClay(captureMode);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::CaptureDrag()
{
    if (ClayUI::IsAnyCaptured())
    {
        return *this;
    }

    ClayUI::capturedElement = props.clay.clayId.id;
    return *this;
}

Element &Element::StopCaptureDrag()
{
    if (ClayUI::capturedElement == props.clay.clayId.id)
    {
        ClayUI::capturedElement = 0;
    }
    return *this;
}

Element &Element::grow()
{
    return size(SizingType::Grow);
}

Element &Element::fit()
{
    return size(SizingType::Fit);
}

void Element::_Begin()
{
    Clay__OpenElement();
    ClayUI::IndentPrintf("Element opened: %s\n", props.id.c_str());

    Clay__AttachId(props.clay.clayId);
    ClayUI::IndentPrintf("Element ID attached: %s\n", props.id.c_str());

    Clay__AttachLayoutConfig(Clay__StoreLayoutConfig(props.clay.layout));
    ClayUI::IndentPrintf("Layout attached\n");

    if (props.clay.activeConfigs & CLAY__ELEMENT_CONFIG_TYPE_RECTANGLE)
    {
        Clay__AttachElementConfig(
            Clay_ElementConfigUnion{
                .rectangleElementConfig = Clay__StoreRectangleElementConfig(
                    (Clay__Clay_RectangleElementConfigWrapper{props.clay.rectangle}).wrapped)},
            CLAY__ELEMENT_CONFIG_TYPE_RECTANGLE);
        ClayUI::IndentPrintf("Rectangle attached\n");
    }

    if (props.clay.activeConfigs & CLAY__ELEMENT_CONFIG_TYPE_BORDER_CONTAINER)
    {
        Clay__AttachElementConfig(
            Clay_ElementConfigUnion{
                .borderElementConfig = Clay__StoreBorderElementConfig(props.clay.border)},
            CLAY__ELEMENT_CONFIG_TYPE_BORDER_CONTAINER);
        ClayUI::IndentPrintf("Border attached\n");
    }

    if (props.clay.activeConfigs & CLAY__ELEMENT_CONFIG_TYPE_SCROLL_CONTAINER)
    {
        Clay__AttachElementConfig(
            Clay_ElementConfigUnion{
                .scrollElementConfig = Clay__StoreScrollElementConfig(props.clay.scroll)},
            CLAY__ELEMENT_CONFIG_TYPE_SCROLL_CONTAINER);
        ClayUI::IndentPrintf("Scroll attached\n");
    }

    if (props.clay.activeConfigs & CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER)
    {
        Clay__AttachElementConfig(
            Clay_ElementConfigUnion{
                .floatingElementConfig = Clay__StoreFloatingElementConfig(props.clay.floating)},
            CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER);
        ClayUI::IndentPrintf("Floating attached\n");
    }

    Clay__ElementPostConfiguration();
    ClayUI::IndentPrintf("Element post configuration\n");
}

void Element::_End()
{
    if (ClayUI::elementDepth == 0)
    {
        throw std::runtime_error("Cannot close root element");
    }

    Clay__CloseElement();
    ClayUI::IndentPrintf("Element closed\n");

    ClayUI::PopStack();
}

bool IsHovered()
{
    return ClayUI::IsHovered();
}

bool HoveredThisFrame()
{
    return ClayUI::HoveredThisFrame();
}

bool IsPressed()
{
    return ClayUI::IsPressed();
}

bool ClickedThisFrame()
{
    return ClayUI::ClickedThisFrame();
}

bool MouseMovedThisFrame()
{
    return ClayUI::MouseMovedThisFrame();
}

bool MouseScrolledThisFrame()
{
    return ClayUI::MouseScrolledThisFrame();
}

bool MouseDraggedThisFrame()
{
    return ClayUI::MouseDraggedThisFrame();
}

ComputedProps Computed()
{
    return ClayUI::GetComputedProperties(*ClayUI::GetCurrentElement());
}

uint32_t ElementIdFromString(std::string id)
{
    return ClayUI::HashId(id).id;
}