#include "clay-text.hpp"
#include "clayui.hpp"

int Text::textId = 0;

Text::Text(const std::string &text)
{
    props.textContent = std::move(text);
    props.clay.activeConfigs |= CLAY__ELEMENT_CONFIG_TYPE_TEXT;
}

Text &Text::color(const Color &color)
{
    props.clay.text.textColor = color;
    return *this;
}

Text &Text::color(const ThemeColor imguiColor)
{
    props.clay.text.textColor = ImGuiColor(imguiColor);
    return *this;
}

Text &Text::fontSize(float size)
{
    props.clay.text.fontSize = size;
    return *this;
}

void Text::_Begin()
{
    Clay__OpenTextElement(
        Clay_String{
            (int)props.textContent.length(),
            ClayUI::AllocateString(props.textContent)},
        Clay__StoreTextElementConfig(
            (Clay__Clay_TextElementConfigWrapper{props.clay.text}).wrapped));
    ClayUI::IndentPrintf("Text added: \"%s\"\n", props.textContent.c_str());
}