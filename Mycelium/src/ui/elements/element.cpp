#pragma once

#define CLAY_IMPLEMENTATION
#include "element.hpp"
#include <stdexcept>

Element *Element::currentElement = nullptr;
std::vector<Element *> Element::elementStack;

void Element::push_stack(Element &element)
{
    element.props.computed.elementDepth = elementStack.size();
    currentElement = &element;
    elementStack.push_back(&element);
}

void Element::pop_stack()
{
    elementStack.pop_back();
    currentElement = elementStack.size() > 0 ? elementStack.back() : nullptr;
}

Element *Element::peek_stack()
{
    return elementStack.size() > 0 ? elementStack.back() : nullptr;
}

void Element::clear_stack()
{
    elementStack.clear();
}

int Element::get_element_depth()
{
    return elementStack.size();
}

Element &Element::get_current_element()
{
    return *Element::currentElement;
}

ComputedProps Element::get_computed_properties()
{
    return ClayState::computedProperties[props.clay.clayId.id];
}

Element::Element(std::string id)
{
    parent = Element::peek_stack();

    if (parent != nullptr)
    {
        props.computed.parentId = parent->props.clay.clayId.id;
    }

    props.id = std::move(id);
    props.clay.clayId = Element::hash_id(props.id);

    Element::push_stack(*this);
}

uint32_t Element::get_current_open_id()
{
    return Clay__GetOpenLayoutElement()->id;
}

Clay_ElementId Element::hash_id(const std::string &id)
{
    return Clay__HashString(Clay_String
    {
        static_cast<int32_t>(id.length()),
        ClayState::allocate_string(id.c_str()),
    },
    0, 0);
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
    props.clay.layout.layoutDirection = to_clay(dir);
    return *this;
}

Element &Element::gap(float gap)
{
    props.clay.layout.childGap = static_cast<uint16_t>(gap);
    return *this;
}

Element &Element::align(AlignmentX x, AlignmentY y)
{
    props.clay.layout.childAlignment.x = to_clay(x);
    props.clay.layout.childAlignment.y = to_clay(y);
    return *this;
}

Element &Element::center_content()
{
    props.clay.layout.childAlignment.x = CLAY_ALIGN_X_CENTER;
    props.clay.layout.childAlignment.y = CLAY_ALIGN_Y_CENTER;
    return *this;
}

Element &Element::center_content_x()
{
    props.clay.layout.childAlignment.x = CLAY_ALIGN_X_CENTER;
    return *this;
}

Element &Element::center_content_y()
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
    this->color(is_pressed() ? pressColor : (is_hovered() ? hoverColor : normalColor));
    return *this;
}

Element &Element::color(const ThemeColor normalColor, const ThemeColor hoverColor, const ThemeColor pressColor)
{
    this->color(is_pressed() ? pressColor : (is_hovered() ? hoverColor : normalColor));
    return *this;
}

Element &Element::corner_radius(float radius)
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

Element &Element::border_width(float width)
{
    props.clay.border.bottom.width = (uint32_t)width;
    props.clay.border.top.width = (uint32_t)width;
    props.clay.border.left.width = (uint32_t)width;
    props.clay.border.right.width = (uint32_t)width;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_BORDER_CONTAINER;
    return *this;
}

Element &Element::border_color(const Color &color)
{
    auto col = Clay_Color{color.r * 255, color.g * 255, color.b * 255, color.a * 255};
    props.clay.border.bottom.color = col;
    props.clay.border.top.color = col;
    props.clay.border.left.color = col;
    props.clay.border.right.color = col;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_BORDER_CONTAINER;
    return *this;
}

Element &Element::gap_border(float width, const Color &color)
{
    props.clay.border.betweenChildren = {(uint32_t)width, {color.r * 255, color.g * 255, color.b * 255, color.a * 255}};
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_BORDER_CONTAINER;
    return *this;
}

Element &Element::gap_border_width(float width)
{
    props.clay.border.betweenChildren.width = (uint32_t)width;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_BORDER_CONTAINER;
    return *this;
}

Element &Element::gap_border_color(const Color &color)
{
    props.clay.border.betweenChildren.color = {color.r * 255, color.g * 255, color.b * 255, color.a * 255};
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_BORDER_CONTAINER;
    return *this;
}

Element &Element::floating_offset(float offsetX, float offsetY)
{
    props.clay.floating.offset.x = offsetX;
    props.clay.floating.offset.y = offsetY;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floating_offset_x(float offsetX)
{
    props.clay.floating.offset.x = offsetX;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floating_offset_y(float offsetY)
{
    props.clay.floating.offset.y = offsetY;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floating_expand(float expandX, float expandY)
{
    props.clay.floating.expand.width = expandX;
    props.clay.floating.expand.height = expandY;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floating_expand_x(float expandX)
{
    props.clay.floating.expand.width = expandX;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floating_expand_y(float expandY)
{
    props.clay.floating.expand.height = expandY;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floating_z_index(int zIndex)
{
    props.clay.floating.zIndex = zIndex;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floating_parent(std::string parentId)
{
    props.clay.floating.parentId = element_id_from_string(parentId);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floating_parent(int parentId)
{
    props.clay.floating.parentId = parentId;
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floating_attach_point(AttachPointType attach)
{
    props.clay.floating.attachment.element = to_clay(attach);
    props.clay.floating.attachment.parent = to_clay(attach);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floating_attach_point_self(AttachPointType attach)
{
    props.clay.floating.attachment.element = to_clay(attach);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}


Element &Element::floating_attach_point_parent(AttachPointType attach)
{
    props.clay.floating.attachment.parent = to_clay(attach);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::floating_pointer_mode(PointerEventMode captureMode)
{
    props.clay.floating.pointerCaptureMode = to_clay(captureMode);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER;
    return *this;
}

Element &Element::capture_drag()
{
    if (Element::is_any_captured())
    {
        return *this;
    }

    ClayState::capturedElement = props.clay.clayId.id;
    return *this;
}

Element &Element::stop_capture_drag()
{
    if (ClayState::capturedElement == props.clay.clayId.id)
    {
        ClayState::capturedElement = 0;
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

void Element::internal_begin()
{
    Clay__OpenElement();

    Clay__AttachId(props.clay.clayId);

    Clay__AttachLayoutConfig(Clay__StoreLayoutConfig(props.clay.layout));

    if (props.clay.activeConfigs & CLAY__ELEMENT_CONFIG_TYPE_RECTANGLE)
    {
        Clay__AttachElementConfig(
            Clay_ElementConfigUnion
        {
            .rectangleElementConfig = Clay__StoreRectangleElementConfig(
            (Clay__Clay_RectangleElementConfigWrapper{props.clay.rectangle}).wrapped)},
        CLAY__ELEMENT_CONFIG_TYPE_RECTANGLE);
    }

    if (props.clay.activeConfigs & CLAY__ELEMENT_CONFIG_TYPE_BORDER_CONTAINER)
    {
        Clay__AttachElementConfig(
            Clay_ElementConfigUnion
        {
            .borderElementConfig = Clay__StoreBorderElementConfig(props.clay.border)},
        CLAY__ELEMENT_CONFIG_TYPE_BORDER_CONTAINER);
    }

    if (props.clay.activeConfigs & CLAY__ELEMENT_CONFIG_TYPE_SCROLL_CONTAINER)
    {
        Clay__AttachElementConfig(
            Clay_ElementConfigUnion
        {
            .scrollElementConfig = Clay__StoreScrollElementConfig(props.clay.scroll)},
        CLAY__ELEMENT_CONFIG_TYPE_SCROLL_CONTAINER);
    }

    if (props.clay.activeConfigs & CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER)
    {
        Clay__AttachElementConfig(
            Clay_ElementConfigUnion
        {
            .floatingElementConfig = Clay__StoreFloatingElementConfig(props.clay.floating)},
        CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER);
    }

    Clay__ElementPostConfiguration();
}

void Element::internal_end()
{
    if (get_element_depth() == 0)
    {
        throw std::runtime_error("Cannot close root element");
    }

    Clay__CloseElement();

    Element::pop_stack();
}

/// @brief Checks if the current element is hovered by the mouse.
bool Element::is_hovered()
{
    if (Clay__booleanWarnings.maxElementsExceeded)
    {
        return false;
    }

    if (is_other_captured())
        return false;

    uint32_t openElement = Element::get_current_element().props.clay.clayId.id;

    for (int32_t i = 0; i < Clay__pointerOverIds.length; ++i)
    {
        auto element = Clay__ElementIdArray_Get(&Clay__pointerOverIds, i);

        if (element->id == openElement)
        {
            ClayState::lastHoveredElement = element->id;
            return true;
        }
    }

    if (ClayState::lastHoveredElement != 0 && ClayState::lastHoveredElement == openElement)
    {
        ClayState::lastHoveredElement = 0;
    }

    return false;
}

/// @brief Checks if the current element is being pressed by the mouse.
bool Element::is_pressed()
{
    return is_hovered() && Clay__pointerInfo.state == CLAY_POINTER_DATA_PRESSED;
}

/// @brief Checks if the current element was clicked this frame.
bool Element::clicked_this_frame()
{
    return is_hovered() && Clay__pointerInfo.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME;
}

/// @brief Checks if the current element started being hovered this frame.
bool Element::hovered_this_frame()
{
    return is_hovered() && (ClayState::lastHoveredElement != 0 && ClayState::lastHoveredElement == get_current_element().props.clay.clayId.id);
}

/// @brief Checks if the mouse moved while over the current element this frame.
bool Element::mouse_moved_this_frame()
{
    return is_hovered() && (ClayState::get_pointer_delta_x() != 0 || ClayState::get_pointer_delta_y() != 0);
}

/// @brief Checks if the mouse was scrolled while over the current element this frame.
bool Element::mouse_scrolled_this_frame()
{
    return is_hovered() && (ClayState::inputs.scrollDeltaX != 0 || ClayState::inputs.scrollDeltaY != 0);
}

/// @brief Checks if the current element is being captured by the mouse.
bool Element::is_self_captured()
{
    if (!Element::is_any_captured())
    {
        return false;
    }

    return ClayState::capturedElement == get_current_element().props.clay.clayId.id;
}

/// @brief Checks if there is any element being captured by the mouse.
bool Element::is_any_captured()
{
    return ClayState::capturedElement != 0;
}

/// @brief Checks if an element other than the current one is being captured by the mouse.
bool Element::is_other_captured()
{
    return Element::is_any_captured() && !Element::is_self_captured();
}

/// @brief Checks if the mouse was dragged while over the current element this frame.
bool Element::mouse_dragged_this_frame()
{
    bool selfCap = is_self_captured();
    bool pressed = is_pressed();
    bool result = selfCap || (pressed && (ClayState::get_pointer_delta_x() != 0 || ClayState::get_pointer_delta_y() != 0));

    // Release capture if mouse is not pressed
    if (selfCap && !ClayState::inputs.pointerDown)
    {
        ClayState::capturedElement = 0;
        return false;
    }

    return result;
}

ComputedProps Element::computed()
{
    return Element::get_current_element().get_computed_properties();
}

uint32_t Element::element_id_from_string(std::string id)
{
    return hash_id(id).id;
}