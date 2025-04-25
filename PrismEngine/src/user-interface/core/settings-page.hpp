#include "core.hpp"
#include "../shared/toggle.hpp"


void draw_settings_page(UIData& state, UIInputs input)
{
    UI(Element("SettingsPage")
       .width(SizingType::Percent, 30)
       .height(SizingType::Fit)
       .color({0,0,0,0.5})
       .padding(10, 10)
       .gap(10)
       .corner_radius(10)
       .direction(FlowDirection::TopToBottom))
    {

    }
}