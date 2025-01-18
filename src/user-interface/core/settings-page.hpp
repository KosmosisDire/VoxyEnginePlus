#include "core.hpp"
#include <user-interface/shared/toggle.hpp>


void settings_page(UIData& state, UIInputs input) {
    UI(Element("SettingsPage")
    .size(SizingType::Percent, 30, 30)
    .color({0,0,0,0.5})
    .padding(10, 10)
    .gap(10)
    .cornerRadius(10)
    .direction(FlowDirection::TopToBottom))
    {
        toggle("Sky Light", &state.renderData->settings.skyLight);
        toggle("Show Steps", &state.renderData->settings.showSteps);
        toggle("Show Normals", &state.renderData->settings.showNormals);
        toggle("Show UVs", &state.renderData->settings.showUvs);
    }
}