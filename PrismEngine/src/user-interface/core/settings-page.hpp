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
       .cornerRadius(10)
       .direction(FlowDirection::TopToBottom))
    {
        toggle("Sky Light", &state.renderData->settings.skyLight);
        toggle("Show Steps", &state.renderData->settings.showSteps);
        toggle("Show Normals", &state.renderData->settings.showNormals);
        toggle("Show Hit Pos", &state.renderData->settings.showHitPos);
        toggle("Show UVs", &state.renderData->settings.showUVs);
        toggle("Show Depth", &state.renderData->settings.showDepth);
        toggle("Show Depth Prepass", &state.renderData->settings.showDepthPrepass);
        toggle("Beam Optimization", &state.renderData->settings.beamOptimization);
    }
}