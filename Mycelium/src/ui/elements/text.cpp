#pragma once
#include "text.hpp"

int Text::textId = 0;

Text::Text(const std::string &text)
{
    props.textContent = std::move(text);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_TEXT;
}

Text &Text::color(const Color &color)
{
    props.clay.text.textColor = to_clay(color);
    return *this;
}

Text &Text::color(const ThemeColor imguiColor)
{
    props.clay.text.textColor = to_clay(ImGuiColor(imguiColor));
    return *this;
}

Text &Text::font_size(float size)
{
    props.clay.text.fontSize = size;
    return *this;
}

void Text::internal_begin()
{
    Clay__OpenTextElement(
        Clay_String
    {
        (int)props.textContent.length(),
                              ClayState::allocate_string(props.textContent)},
    Clay__StoreTextElementConfig(
        (Clay__Clay_TextElementConfigWrapper{props.clay.text}).wrapped));
}