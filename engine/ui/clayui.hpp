#pragma once

#define CLAY_IMPLEMENTATION
#include "clay-types.hpp"
#include "clay.h"
#include "element-clayui-forward.hpp"
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

// Forward declare Element::_Begin() and Element::_End()
void _Element_Begin(Element &elem);
void _Element_End();

#define UI(element)                                                   \
    for (int CONCAT(latch_, __LINE__) = (_Element_Begin(element), 0); \
         CONCAT(latch_, __LINE__) < 1;                                \
         ++CONCAT(latch_, __LINE__), _Element_End())

#define UITEXT(element) element._Begin();

struct string_alloc
{
    char *str;
    int access_count;
};

class ClayUI
{
  private:
    static int elementDepth;
    static Element *currentElement;
    static uint32_t lastHoveredElement;
    static uint32_t capturedElement;
    static bool initialized;
    static UIInputs inputs;
    static std::unordered_map<std::string, string_alloc> currentStrings;
    static std::unordered_map<uint32_t, ComputedProps> computedProperties;
    static std::vector<Element *> elementStack;
    static float lastPointerX;
    static float lastPointerY;
    static float pointerDeltaX;
    static float pointerDeltaY;

    // how long the frame took to layout and dispatch render calls
    static double uiFrameTime;
    static std::chrono::time_point<std::chrono::high_resolution_clock> frameStart;

    // Private constructor/assignment operators to prevent instantiation
    ClayUI() = delete;
    ClayUI(const ClayUI &) = delete;
    ClayUI &operator=(const ClayUI &) = delete;

    static void IndentPrintf(const char *format, ...);
    static void HandleClayErrors(Clay_ErrorData errorData);
    static void Initialize(UIInputs input);
    static void SetInputs(UIInputs input);
    static void SetComputedProps(ComputedProps *props, Clay_RenderCommand *command);
    static void FillComputedProperties(Clay_RenderCommandArray commands);
    static void Render();
    static void BeginFrame(UIInputs input);
    static void PushStack(Element &element);
    static void PopStack();
    static Element *PeekStack();

    static char *AllocateString(const std::string &str);

  public:
    static Element *GetCurrentElement();
    static uint32_t GetCurrentOpenId();

    // @brief Gets the computed properties of an element.
    static ComputedProps GetComputedProperties(Element &element);

    // @brief Hashes a string to a Clay_ElementId.
    static Clay_ElementId HashId(const std::string &id);

    static float GetPointerDeltaX();
    static float GetPointerDeltaY();

    /// @brief Checks if the current element is hovered by the mouse.
    static bool IsHovered();
    /// @brief Checks if the current element is being pressed by the mouse.
    static bool IsPressed();
    /// @brief Checks if the current element was clicked this frame.
    static bool ClickedThisFrame();
    /// @brief Checks if the current element started being hovered this frame.
    static bool HoveredThisFrame();
    /// @brief Checks if the mouse moved while over the current element this frame.
    static bool MouseMovedThisFrame();
    /// @brief Checks if the mouse was scrolled while over the current element this frame.
    static bool MouseScrolledThisFrame();
    /// @brief Checks if the mouse was dragged while over the current element this frame.
    static bool MouseDraggedThisFrame();
    /// @brief Checks if the current element is being captured by the mouse.
    static bool IsSelfCaptured();
    /// @brief Checks if there is any element being captured by the mouse.
    static bool IsAnyCaptured();
    /// @brief Checks if an element other than the current one is being captured by the mouse.
    static bool IsOtherCaptured();

    static void FreeStrings();
    static void FreeAllStrings();

    template <typename T>
    static void Update(T &state, UIInputs input, std::function<void(T &, UIInputs)> build_ui);

    friend struct Text;
    friend struct Element;
};

template <typename T>
void ClayUI::Update(T &state, UIInputs input, std::function<void(T &, UIInputs)> build_ui)
{
    // frameStart = std::chrono::high_resolution_clock::now();
    BeginFrame(input);
    build_ui(state, input);
    Render();
    // uiFrameTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - frameStart).count();
    // printf("UI frame time: %f\n", uiFrameTime);
}