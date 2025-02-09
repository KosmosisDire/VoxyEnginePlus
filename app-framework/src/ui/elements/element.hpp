#pragma once
#include "helper/clay-state.hpp"
#include "helper/clay-types.hpp"
#include <clay.h>
#include <concepts>
#include <string>
#include <type_traits>
#include <vector>

struct Element;

using BasicCallback = void(Element &, ComputedProps, UIInputs);
template <typename T>
using DataCallback = void(Element &, ComputedProps, UIInputs, T &);

template <typename C>
concept BasicCallable = requires(C c, Element &el, ComputedProps props, UIInputs inputs) {
    {
        c(el, props, inputs)
    }
    -> std::same_as<void>;
};

template <typename C, typename T>
concept DataCallable = requires(C c, Element &el, ComputedProps props, UIInputs inputs, T &data) {
    {
        c(el, props, inputs, data)
    }
    -> std::same_as<void>;
};

#define UI(element)                                            \
    for (int CONCAT(latch_, __LINE__) = (element._Begin(), 0); \
         CONCAT(latch_, __LINE__) < 1;                         \
         ++CONCAT(latch_, __LINE__), Element::_End())

#define UITEXT(element) element._Begin();

struct Element
{
    friend class ClayUI;

  private:
    static Element *currentElement;
    static std::vector<Element *> elementStack;
    static void PushStack(Element &element);
    static void PopStack();
    static Element *PeekStack();
    static void ClearStack();
    static int GetElementDepth();
    static bool IsSelfCaptured();
    static bool IsAnyCaptured();
    static bool IsOtherCaptured();

    /// @return The presistent ID of the currently open element.
    static uint32_t GetCurrentOpenId();
    /// @brief Hashes a string to a Clay_ElementId.
    static Clay_ElementId HashId(const std::string &id);

  public:
    static Element &GetCurrentElement();
    /// @brief Get the ID associated with the element id string.
    static uint32_t ElementIdFromString(std::string id);
    explicit Element(std::string id);
    ElementProps props;
    Element *parent = nullptr;

    // Builder methods for element properties
    Element &size(SizingType type = SizingType::Fixed, float width = 100, float height = 100);
    Element &size(float width, float height);
    Element &width(SizingType type, float width = 100);
    Element &height(SizingType type, float height = 100);
    Element &width(float width);
    Element &height(float height);
    Element &direction(FlowDirection dir);
    Element &gap(float gap);
    Element &align(AlignmentX x, AlignmentY y);
    Element &centerContent();
    Element &centerContentX();
    Element &centerContentY();
    Element &padding(float x, float y);
    Element &color(const Color &color);
    Element &color(const ThemeColor imguiColor);
    Element &color(const Color &normalColor, const Color &hoverColor, const Color &pressColor);
    Element &color(const ThemeColor normalColor, const ThemeColor hoverColor, const ThemeColor pressColor);
    Element &cornerRadius(float radius);
    Element &scroll(bool horizontal = true, bool vertical = true);

    Element &grow();
    Element &fit();

    Element &border(float width, const Color &color);
    Element &borderWidth(float width);
    Element &borderColor(const Color &color);
    Element &gapBorder(float width, const Color &color);
    Element &gapBorderWidth(float width);
    Element &gapBorderColor(const Color &color);

    Element &floatingOffset(float offsetX, float offsetY);
    Element &floatingOffsetX(float offsetX);
    Element &floatingOffsetY(float offsetY);
    Element &floatingExpand(float expandX, float expandY);
    Element &floatingExpandX(float expandX);
    Element &floatingExpandY(float expandY);
    Element &floatingZIndex(int zIndex);
    Element &floatingParent(std::string parentId);
    Element &floatingParent(int parentId);
    Element &floatingAttachPoint(AttachPointType attach);
    Element &floatingAttachPointSelf(AttachPointType attach);
    Element &floatingAttachPointParent(AttachPointType attach);
    Element &floatingPointerMode(PointerEventMode pointerMode);

    // @brief Gets the computed properties of an element.
    ComputedProps GetComputedProperties();

    // Event handlers
    Element &CaptureDrag();
    Element &StopCaptureDrag();

    /// Sets a hover callback that runs every frame while the mouse is over the element. Takes a generic user data parameter of any type.
    ///
    /// Usage with lambda:
    ///```cpp
    ///     Element("Example").OnHovering(userData,
    ///         [](Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
    ///         {
    ///             // Handle hover state
    ///         });
    ///```
    ///
    /// Usage with external function:
    ///```cpp
    ///     void HandleHovering(Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
    ///     {
    ///         // Handle hover state
    ///     }
    ///
    ///     Element("Example").OnHovering(userData, HandleHovering);
    ///```
    template <typename TUserData, DataCallable<TUserData> Callable>
    Element &OnHovering(TUserData &userData, Callable &&callback);

    /// Sets a hover callback that runs every frame while the mouse is over the element.
    ///
    /// Usage with lambda:
    ///```cpp
    ///     Element("Example").OnHovering(
    ///         [](Element& el, ComputedProps props, UIInputs inputs)
    ///         {
    ///             // Handle hover state
    ///         });
    ///```
    ///
    /// Usage with external function:
    ///```cpp
    ///     void HandleHovering(Element& el, ComputedProps props, UIInputs inputs)
    ///     {
    ///         // Handle hover state
    ///     }
    ///
    ///     Element("Example").OnHovering(HandleHovering);
    ///```
    template <BasicCallable Callable>
    Element &OnHovering(Callable &&callback);

    /// Sets a callback that runs every frame while the mouse button is held down over the element. Takes a generic user data parameter of any type.
    ///
    /// Usage with lambda:
    ///```cpp
    ///     Element("Example").OnPressing(userData,
    ///         [](Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
    ///         {
    ///             // Handle press state
    ///         });
    ///```
    ///
    /// Usage with external function:
    ///```cpp
    ///     void HandlePressing(Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
    ///     {
    ///         // Handle press state
    ///     }
    ///
    ///     Element("Example").OnPressing(userData, HandlePressing);
    ///```
    template <typename TUserData, DataCallable<TUserData> Callable>
    Element &OnPressing(TUserData &userData, Callable &&callback);

    /// Sets a callback that runs every frame while the mouse button is held down over the element.
    ///
    /// Usage with lambda:
    ///```cpp
    ///     Element("Example").OnPressing(
    ///         [](Element& el, ComputedProps props, UIInputs inputs)
    ///         {
    ///             // Handle press state
    ///         });
    ///```
    ///
    /// Usage with external function:
    ///```cpp
    ///     void HandlePressing(Element& el, ComputedProps props, UIInputs inputs)
    ///     {
    ///         // Handle press state
    ///     }
    ///
    ///     Element("Example").OnPressing(HandlePressing);
    ///```
    template <BasicCallable Callable>
    Element &OnPressing(Callable &&callback);

    /// Sets a callback that runs once when the mouse first enters the element's bounds. Takes a generic user data parameter of any type.
    ///
    /// Usage with lambda:
    ///```cpp
    ///     Element("Example").OnHover(userData,
    ///         [](Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
    ///         {
    ///             // Handle hover enter
    ///         });
    ///```
    ///
    /// Usage with external function:
    ///```cpp
    ///     void HandleHover(Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
    ///     {
    ///         // Handle hover enter
    ///     }
    ///
    ///     Element("Example").OnHover(userData, HandleHover);
    ///```
    template <typename TUserData, DataCallable<TUserData> Callable>
    Element &OnHover(TUserData &userData, Callable &&callback);

    /// Sets a callback that runs once when the mouse first enters the element's bounds.
    ///
    /// Usage with lambda:
    ///```cpp
    ///     Element("Example").OnHover(
    ///         [](Element& el, ComputedProps props, UIInputs inputs)
    ///         {
    ///             // Handle hover enter
    ///         });
    ///```
    ///
    /// Usage with external function:
    ///```cpp
    ///     void HandleHover(Element& el, ComputedProps props, UIInputs inputs)
    ///     {
    ///         // Handle hover enter
    ///     }
    ///
    ///     Element("Example").OnHover(HandleHover);
    ///```
    template <BasicCallable Callable>
    Element &OnHover(Callable &&callback);

    /// Sets a callback that runs once when the mouse clicks the element. Takes a generic user data parameter of any type.
    ///
    /// Usage with lambda:
    ///```cpp
    ///     Element("Example").OnClick(userData,
    ///         [](Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
    ///         {
    ///             // Handle click
    ///         });
    ///```
    ///
    /// Usage with external function:
    ///```cpp
    ///     void HandleClick(Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
    ///     {
    ///         // Handle click
    ///     }
    ///
    ///     Element("Example").OnClick(userData, HandleClick);
    ///```
    template <typename TUserData, DataCallable<TUserData> Callable>
    Element &OnClick(TUserData &userData, Callable &&callback);

    /// Sets a callback that runs once when the mouse clicks the element.
    ///
    /// Usage with lambda:
    ///```cpp
    ///     Element("Example").OnClick(
    ///         [](Element& el, ComputedProps props, UIInputs inputs)
    ///         {
    ///             // Handle click
    ///         });
    ///```
    ///
    /// Usage with external function:
    ///```cpp
    ///     void HandleClick(Element& el, ComputedProps props, UIInputs inputs)
    ///     {
    ///         // Handle click
    ///     }
    ///
    ///     Element("Example").OnClick(HandleClick);
    ///```
    template <BasicCallable Callable>
    Element &OnClick(Callable &&callback);

    /// Sets a callback that runs each frame when the mouse moves while over the element. Takes a generic user data parameter of any type.
    ///
    /// Usage with lambda:
    ///```cpp
    ///     Element("Example").OnMouseMove(userData,
    ///         [](Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
    ///         {
    ///             // Handle mouse movement
    ///         });
    ///```
    ///
    /// Usage with external function:
    ///```cpp
    ///     void HandleMouseMove(Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
    ///     {
    ///         // Handle mouse movement
    ///     }
    ///
    ///     Element("Example").OnMouseMove(userData, HandleMouseMove);
    ///```
    template <typename TUserData, DataCallable<TUserData> Callable>
    Element &OnMouseMove(TUserData &userData, Callable &&callback);

    /// Sets a callback that runs each frame when the mouse moves while over the element.
    ///
    /// Usage with lambda:
    ///```cpp
    ///     Element("Example").OnMouseMove(
    ///         [](Element& el, ComputedProps props, UIInputs inputs)
    ///         {
    ///             // Handle mouse movement
    ///         });
    ///```
    ///
    /// Usage with external function:
    ///```cpp
    ///     void HandleMouseMove(Element& el, ComputedProps props, UIInputs inputs)
    ///     {
    ///         // Handle mouse movement
    ///     }
    ///
    ///     Element("Example").OnMouseMove(HandleMouseMove);
    ///```
    template <BasicCallable Callable>
    Element &OnMouseMove(Callable &&callback);

    /// Sets a callback that runs each frame when the mouse scrolls while over the element. Takes a generic user data parameter of any type.
    ///
    /// Usage with lambda:
    ///```cpp
    ///     Element("Example").OnScroll(userData,
    ///         [](Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
    ///         {
    ///             // Handle scroll
    ///         });
    ///```
    ///
    /// Usage with external function:
    ///```cpp
    ///     void HandleScroll(Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
    ///     {
    ///         // Handle scroll
    ///     }
    ///
    ///     Element("Example").OnScroll(userData, HandleScroll);
    ///```
    template <typename TUserData, DataCallable<TUserData> Callable>
    Element &OnScroll(TUserData &userData, Callable &&callback);

    /// Sets a callback that runs each frame when the mouse scrolls while over the element.
    ///
    /// Usage with lambda:
    ///```cpp
    ///     Element("Example").OnScroll(
    ///         [](Element& el, ComputedProps props, UIInputs inputs)
    ///         {
    ///             // Handle scroll
    ///         });
    ///```
    ///
    /// Usage with external function:
    ///```cpp
    ///     void HandleScroll(Element& el, ComputedProps props, UIInputs inputs)
    ///     {
    ///         // Handle scroll
    ///     }
    ///
    ///     Element("Example").OnScroll(HandleScroll);
    ///```
    template <BasicCallable Callable>
    Element &OnScroll(Callable &&callback);

    /// Sets a callback that runs each frame when the element is being dragged (clicked and moved). Takes a generic user data parameter of any type.
    ///
    /// Usage with lambda:
    ///```cpp
    ///     Element("Example").OnDrag(userData,
    ///         [](Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
    ///         {
    ///             // Handle drag
    ///         });
    ///```
    ///
    /// Usage with external function:
    ///```cpp
    ///     void HandleDrag(Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
    ///     {
    ///         // Handle drag
    ///     }
    ///
    ///     Element("Example").OnDrag(userData, HandleDrag);
    ///```
    template <typename TUserData, DataCallable<TUserData> Callable>
    Element &OnDrag(TUserData &userData, Callable &&callback);

    /// Sets a callback that runs each frame when the element is being dragged (clicked and moved).
    ///
    /// Usage with lambda:
    ///```cpp
    ///     Element("Example").OnDrag(
    ///         [](Element& el, ComputedProps props, UIInputs inputs)
    ///         {
    ///             // Handle drag
    ///         });
    ///```
    ///
    /// Usage with external function:
    ///```cpp
    ///     void HandleDrag(Element& el, ComputedProps props, UIInputs inputs)
    ///     {
    ///         // Handle drag
    ///     }
    ///
    ///     Element("Example").OnDrag(HandleDrag);
    ///```
    template <BasicCallable Callable>
    Element &OnDrag(Callable &&callback);

    /// @brief Checks if the current element is hovered by the mouse.
    static bool IsHovered();
    /// @brief Checks if the current element is being pressed by the mouse.
    static bool HoveredThisFrame();
    /// @brief Checks if the current element is being pressed by the mouse.
    static bool IsPressed();
    /// @brief Checks if the current element was clicked this frame.
    static bool ClickedThisFrame();
    /// @brief Checks if the current element started being hovered this frame.
    static bool MouseMovedThisFrame();
    /// @brief Checks if the mouse was scrolled while over the current element this frame.
    static bool MouseScrolledThisFrame();
    /// @brief Checks if the mouse was dragged while over the current element this frame.
    static bool MouseDraggedThisFrame();
    /// @brief Get the computed properties of the current element.
    static ComputedProps Computed();

    void _Begin();
    static void _End();
};

/// @brief Sets a callback to run every frame the mouse is hovering over the element.
template <BasicCallable Callable>
Element &Element::OnHovering(Callable &&callback)
{
    if (IsHovered())
    {
        callback(*this, Computed(), ClayState::inputs);
    }
    return *this;
}

/// @brief Sets a callback to run every frame the mouse is hovering over the element. Takes a generic user data parameter.
template <typename TUserData, DataCallable<TUserData> Callable>
Element &Element::OnHovering(TUserData &userData, Callable &&callback)
{
    if (IsHovered())
    {
        callback(*this, Computed(), ClayState::inputs, userData);
    }
    return *this;
}

/// @brief Sets a callback to run every frame the mouse is pressing the element.
template <BasicCallable Callable>
Element &Element::OnPressing(Callable &&callback)
{
    if (IsPressed())
    {
        callback(*this, Computed(), ClayState::inputs);
    }
    return *this;
}

/// @brief Sets a callback to run every frame the mouse is pressing the element.
template <typename TUserData, DataCallable<TUserData> Callable>
Element &Element::OnPressing(TUserData &userData, Callable &&callback)
{
    if (IsPressed())
    {
        callback(*this, Computed(), ClayState::inputs, userData);
    }
    return *this;
}

/// @brief Sets a callback to run once on the frame the mouse enters the element's bounds.
template <BasicCallable Callable>
Element &Element::OnHover(Callable &&callback)
{
    if (HoveredThisFrame())
    {
        callback(*this, Computed(), ClayState::inputs);
    }
    return *this;
}

/// @brief Sets a callback to run once on the frame the mouse enters the element's bounds. Takes a generic user data parameter.
template <typename TUserData, DataCallable<TUserData> Callable>
Element &Element::OnHover(TUserData &userData, Callable &&callback)
{
    if (HoveredThisFrame())
    {
        callback(*this, Computed(), ClayState::inputs, userData);
    }
    return *this;
}

/// @brief Sets a callback to run once on the frame the mouse clicks the element.
template <BasicCallable Callable>
Element &Element::OnClick(Callable &&callback)
{
    if (ClickedThisFrame())
    {
        callback(*this, Computed(), ClayState::inputs);
    }
    return *this;
}

/// @brief Sets a callback to run once on the frame the mouse clicks the element. Takes a generic user data parameter.
template <typename TUserData, DataCallable<TUserData> Callable>
Element &Element::OnClick(TUserData &userData, Callable &&callback)
{
    if (ClickedThisFrame())
    {
        callback(*this, Computed(), ClayState::inputs, userData);
    }
    return *this;
}

/// @brief Sets a callback to run each frame the mouse moves while over the element.
template <BasicCallable Callable>
Element &Element::OnMouseMove(Callable &&callback)
{
    if (MouseMovedThisFrame())
    {
        callback(*this, Computed(), ClayState::inputs);
    }
    return *this;
}

/// @brief Sets a callback to run each frame the mouse moves while over the element.
template <typename TUserData, DataCallable<TUserData> Callable>
Element &Element::OnMouseMove(TUserData &userData, Callable &&callback)
{
    if (MouseMovedThisFrame())
    {
        callback(*this, Computed(), ClayState::inputs, userData);
    }
    return *this;
}

/// @brief Sets a callback to run each frame the mouse scrolls while over the element.
template <BasicCallable Callable>
Element &Element::OnScroll(Callable &&callback)
{
    if (MouseScrolledThisFrame())
    {
        callback(*this, Computed(), ClayState::inputs);
    }
    return *this;
}

/// @brief Sets a callback to run each frame the mouse scrolls while over the element.
template <typename TUserData, DataCallable<TUserData> Callable>
Element &Element::OnScroll(TUserData &userData, Callable &&callback)
{
    if (MouseScrolledThisFrame())
    {
        callback(*this, Computed(), ClayState::inputs, userData);
    }
    return *this;
}

/// @brief Sets a callback to run each frame the element is clicked and dragged.
template <BasicCallable Callable>
Element &Element::OnDrag(Callable &&callback)
{
    if (MouseDraggedThisFrame())
    {
        callback(*this, Computed(), ClayState::inputs);
    }
    return *this;
}

/// @brief Sets a callback to run each frame the element is clicked and dragged.
template <typename TUserData, DataCallable<TUserData> Callable>
Element &Element::OnDrag(TUserData &userData, Callable &&callback)
{
    if (MouseDraggedThisFrame())
    {
        callback(*this, Computed(), ClayState::inputs, userData);
    }
    return *this;
}