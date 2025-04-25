#pragma once

#include "clay-types.hpp"
#include <chrono>
#include <string>
#include <unordered_map>

// Debug configuration
#define UI_DEBUG 0

// UI macro helpers
#define CONCAT_HELPER(x, y) x##y
#define CONCAT(x, y) CONCAT_HELPER(x, y)

struct StringAlloc
{
    char* str;
    int access_count;
};

class ClayState
{
    private:
        static uint32_t lastHoveredElement;
        static uint32_t capturedElement;
        static std::unordered_map<std::string, StringAlloc> currentStrings;
        static std::unordered_map<uint32_t, ComputedProps> computedProperties;

        static UIInputs inputs;
        static float lastPointerX;
        static float lastPointerY;

        static float pointerDeltaX;
        static float pointerDeltaY;

        // how long the frame took to layout and dispatch render calls
        static double uiFrameTime;
        static std::chrono::time_point<std::chrono::high_resolution_clock> frameStart;

        // Private constructor/assignment operators to prevent instantiation
        ClayState() = delete;
        ClayState(const ClayState &) = delete;
        ClayState &operator=(const ClayState &) = delete;

        static void set_computed_props(ComputedProps *props, Clay_RenderCommand *command);
        static void fill_computed_properties(Clay_RenderCommandArray commands);
        static char* allocate_string(const std::string &str);

    public:
        static float get_pointer_delta_x();
        static float get_pointer_delta_y();
        static void free_strings();
        static void free_all_strings();

        friend struct Text;
        friend struct Element;
        friend class ClayUI;
};