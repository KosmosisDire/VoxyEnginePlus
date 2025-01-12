#include "core/core.hpp"
#include "core/menu-page.hpp"
#include "core/settings-page.hpp"


void init_root_ui(UIData& data, UIInputs input) {
    data.pages = {
        {PageID::MENU, false, menu_page},
        {PageID::SETTINGS, true,  settings_page},
    };

    UI(Element("Root")
    .grow()
    .align(AlignmentX::Center, AlignmentY::Center))
    {
        for (const auto& page : data.pages) {
            if (page.isActive) {
                page.init_page(data, input);
            }
        }
    }
}