#include <clayui/core.hpp>
#include <daxa/daxa.hpp>


// feel free to delete this file. Just a temporary ui toggle

void toggle(const char* label, bool* value) {
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
        .color(*value ? ThemeColor::ButtonActive : ThemeColor::FrameBg)
        .cornerRadius(12)
        .OnClick([value](Element &el, ComputedProps props, UIInputs input)
        {
            *value = !*value;
        }))
        {
            // Toggle handle
            UI(Element("Handle" + string(label))
            .width(20)
            .height(20)
            .floatingOffset(*value ? 27 : 2, 2)
            .floatingZIndex(-100)
            .floatingPointerMode(PointerEventMode::Passthrough)
            .color(ThemeColor::TextColor)
            .cornerRadius(10)){}
        }
    }
}

// version that works on an int by recasting to bool
void toggle(const char* label, daxa_u32* value) {
    toggle(label, (bool*)value);
}