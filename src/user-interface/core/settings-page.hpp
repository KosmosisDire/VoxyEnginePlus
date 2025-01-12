#include "core.hpp"


void settings_page(UIData& state, UIInputs input) {
    UI(Element("SettingsPage")
    .size(SizingType::Percent, 100, 100)
    .color(ThemeColor::WindowBg)
    .padding(10, 10)
    .gap(10)
    .cornerRadius(10)
    .direction(FlowDirection::TopToBottom))
    {

    }
}