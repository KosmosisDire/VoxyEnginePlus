#pragma once

// clang-format off
#include <fmt/core.h>
#include <string>
#include <clayui/core.hpp>
#include <daxa/daxa.hpp>
#include <algorithm>
#include <vector>

using namespace daxa::types;
using namespace std;

enum class PageID {
    MENU = 0,
    SETTINGS = 1
};

struct UIData;

struct Page {         
    PageID pageID;  
    bool isActive;             
    std::function<void(UIData& data, UIInputs input)> init_page; 
};

struct UIData {
    float time;
    daxa_f32vec3 backgroundColor;
    int selectedPage = 0;
    bool mouseIsActive = true;
    std::vector<Page> pages;
};

