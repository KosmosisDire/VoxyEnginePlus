#include "core.hpp"


void draw_menu_page(UIData& state, UIInputs input) {
    UI(Element("MenuPage")
    .size(SizingType::Percent, 100, 100)
    .color(ThemeColor::WindowBg)
    .padding(10, 10)
    .gap(10)
    .cornerRadius(10)
    .direction(FlowDirection::TopToBottom))
    {

    }
}