#pragma once

#define CLAY_IMPLEMENTATION
#include "clay-types.hpp"
#include "clay.h"
#include <chrono>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

// Debug configuration
#define UI_DEBUG 0

// UI macro helpers
#define CONCAT_HELPER(x, y) x##y
#define CONCAT(x, y) CONCAT_HELPER(x, y)

struct string_alloc
{
    char *str;
    int access_count;
};

class ClayState
{
  private:
    static uint32_t lastHoveredElement;
    static uint32_t capturedElement;
    static std::unordered_map<std::string, string_alloc> currentStrings;
    static std::unordered_map<uint32_t, ComputedProps> computedProperties;
    static float lastPointerX;
    static float lastPointerY;

    static float pointerDeltaX;
    static float pointerDeltaY;
    static bool initialized;
    static UIInputs inputs;
    // how long the frame took to layout and dispatch render calls
    static double uiFrameTime;
    static std::chrono::time_point<std::chrono::high_resolution_clock> frameStart;

    // Private constructor/assignment operators to prevent instantiation
    ClayState() = delete;
    ClayState(const ClayState &) = delete;
    ClayState &operator=(const ClayState &) = delete;

    static void HandleClayErrors(Clay_ErrorData errorData);
    static void Initialize(UIInputs input);
    static void SetInputs(UIInputs input);
    static void SetComputedProps(ComputedProps *props, Clay_RenderCommand *command);
    static void FillComputedProperties(Clay_RenderCommandArray commands);
    static char *AllocateString(const std::string &str);

  public:
    static float GetPointerDeltaX();
    static float GetPointerDeltaY();
    static void FreeStrings();
    static void FreeAllStrings();

    friend struct Text;
    friend struct Element;
};