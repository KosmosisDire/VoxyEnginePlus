#include "core/core.hpp"
#include "core/menu-page.hpp"
#include "core/settings-page.hpp"
// clang-format off

void init_root_ui(UIData& data, UIInputs input)
{
    
    data.pages = {
        {PageID::MENU, false, menu_page},
        {PageID::SETTINGS, true,  settings_page},
    };

    UI(Element("Root")
    .grow()
    .centerContent())
    {
        // display frame time in top corner
        UI(Element("FloatingCorner")
        .floatingAttachPointParent(AttachPointType::RightTop, AttachPointType::RightTop)
        .floatingAttachPointSelf(AttachPointType::RightTop, AttachPointType::RightTop)
        .floatingOffset(-10, 10))
        {
            UITEXT(Text(fmt::format("Frame Time: {:.2f}ms", input.deltaTime * 1000.0f))
            .color(ThemeColor::TextColor)
            .fontSize(16));
        }

        if (!data.mouseIsActive) continue; // is this how this should be used?

        // display the current page
        for (const auto& page : data.pages)
        {
            if (page.isActive)
            {
                page.init_page(data, input);
            }
        }

    }
}