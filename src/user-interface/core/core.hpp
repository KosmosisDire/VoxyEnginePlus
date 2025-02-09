#pragma once

// clang-format off
#include <fmt/core.h>
#include <string>
#include <ui/core.hpp>
#include <daxa/daxa.hpp>
#include <algorithm>
#include <vector>
#include <vox/shaders/shared.inl>

using namespace daxa::types;
using namespace std;

struct UIData;

struct Page
{         
    bool isActive;             
    std::function<void(UIData& data, UIInputs input)> render_page; 
};

struct UIData {
    float time = 0.0;
    int selectedPage = 0;
    bool mouseIsActive = true; // Does this mean that the mouse is captured or not captured? Is this meant to be used to hide or show the UI?
    std::unordered_map<std::string, Page> pages; // name -> page
    RenderData *renderData;
};

