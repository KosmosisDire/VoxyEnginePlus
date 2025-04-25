#include <ui/core.hpp>
#include <daxa/daxa.hpp>


// feel free to delete this file. Just a temporary ui toggle

void toggle(const char* label, bool* value)
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
               .font_size(16));

        // Toggle switch
        UI(Element("Toggle" + string(label))
           .width(50)
           .height(24)
           .color(*value ? ThemeColor::ButtonActive : ThemeColor::FrameBg)
           .corner_radius(12)
           .on_click([value](Element &el, ComputedProps props, UIInputs input)
        {
            *value = !*value;
        }))

        {
            // Toggle handle
            UI(Element("Handle" + string(label))
               .width(20)
               .height(20)
               .floating_offset(*value ? 27 : 2, 2)
               .floating_z_index(-100)
               .floating_pointer_mode(PointerEventMode::Passthrough)
               .color(ThemeColor::TextColor)
               .corner_radius(10)) {}
        }
    }
}

// version that works on an int by recasting to bool
void toggle(const char* label, daxa_u32* value)
{
    toggle(label, (bool*)value);
}