#pragma once
#include "clay-state.hpp"
#include "clay-types.hpp"

#include <engine/apis/Fonts.hpp>
#include <stdarg.h>
#include <stdio.h>

// Static member definitions
uint32_t ClayState::lastHoveredElement = 0;
uint32_t ClayState::capturedElement = 0;
std::unordered_map<std::string, string_alloc> ClayState::currentStrings;
std::unordered_map<uint32_t, ComputedProps> ClayState::computedProperties;

UIInputs ClayState::inputs = {};

float ClayState::lastPointerX = 0;
float ClayState::lastPointerY = 0;
float ClayState::pointerDeltaX = 0;
float ClayState::pointerDeltaY = 0;

double ClayState::uiFrameTime;
std::chrono::time_point<std::chrono::high_resolution_clock> ClayState::frameStart;

void ClayState::SetComputedProps(ComputedProps *props, Clay_RenderCommand *command)
{
    auto bb = command->boundingBox;
    props->width = bb.width;
    props->height = bb.height;
    props->x = bb.x;
    props->y = bb.y;
    props->parent = &ClayState::computedProperties[props->parentId];
}

void ClayState::FillComputedProperties(Clay_RenderCommandArray commands)
{
    ClayState::computedProperties.clear();

    for (int i = 0; i < commands.length; ++i)
    {
        Clay_RenderCommand *command = Clay_RenderCommandArray_Get(&commands, i);
        SetComputedProps(&ClayState::computedProperties[command->id], command);
    }
}

float ClayState::GetPointerDeltaX()
{
    return pointerDeltaX;
}

float ClayState::GetPointerDeltaY()
{
    return pointerDeltaY;
}

char* ClayState::AllocateString(const std::string &str)
{
    if (currentStrings.find(str) != currentStrings.end())
    {
        currentStrings[str].access_count += 2;
        return currentStrings[str].str;
    }

    char* newStr = (char*)malloc(str.length() + 1);
    strcpy(newStr, str.c_str());
    currentStrings[str] = {newStr, 10};
    return newStr;
}

void ClayState::FreeStrings()
{
    for (const auto &[key, value] : currentStrings)
    {
        if (value.access_count < -10)
        {
            free(value.str);
            currentStrings.erase(key);
            break;
        }
        else
        {
            currentStrings[key].access_count--;
        }
    }
}

void ClayState::FreeAllStrings()
{
    for (const auto &[key, value] : currentStrings)
    {
        free(value.str);
    }

    currentStrings.clear();
}