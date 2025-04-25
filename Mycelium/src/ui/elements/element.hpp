#pragma once
#include "helper/clay-state.hpp"
#include "helper/clay-types.hpp"
#include <clay.h>
#include <concepts>
#include <string>
#include <vector>

struct Element;

using BasicCallback = void(Element &, ComputedProps, UIInputs);
template <typename T>
using DataCallback = void(Element &, ComputedProps, UIInputs, T &);

template <typename C>
concept BasicCallable = requires(C c, Element &el, ComputedProps props, UIInputs inputs)
{
    {
        c(el, props, inputs)
    }

    -> std::same_as<void>;
};

template <typename C, typename T>
concept DataCallable = requires(C c, Element &el, ComputedProps props, UIInputs inputs, T &data)
{
    {
        c(el, props, inputs, data)
    }

    -> std::same_as<void>;
};

#define UI(element)                                            \
    for (int CONCAT(latch_, __LINE__) = (element.internal_begin(), 0); \
         CONCAT(latch_, __LINE__) < 1;                         \
         ++CONCAT(latch_, __LINE__), Element::internal_end())

#define UITEXT(element) element.internal_begin();

struct Element
{
        friend class ClayUI;

    private:
        static Element *currentElement;
        static std::vector<Element *> elementStack;
        static void push_stack(Element &element);
        static void pop_stack();
        static Element *peek_stack();
        static void clear_stack();
        static int get_element_depth();
        static bool is_self_captured();
        static bool is_any_captured();
        static bool is_other_captured();

        /// @return The presistent ID of the currently open element.
        static uint32_t get_current_open_id();
        /// @brief Hashes a string to a Clay_ElementId.
        static Clay_ElementId hash_id(const std::string &id);

    public:
        static Element &get_current_element();
        /// @brief Get the ID associated with the element id string.
        static uint32_t element_id_from_string(std::string id);
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
        Element &center_content();
        Element &center_content_x();
        Element &center_content_y();
        Element &padding(float x, float y);
        Element &color(const Color &color);
        Element &color(const ThemeColor imguiColor);
        Element &color(const Color &normalColor, const Color &hoverColor, const Color &pressColor);
        Element &color(const ThemeColor normalColor, const ThemeColor hoverColor, const ThemeColor pressColor);
        Element &corner_radius(float radius);
        Element &scroll(bool horizontal = true, bool vertical = true);

        Element &grow();
        Element &fit();

        Element &border(float width, const Color &color);
        Element &border_width(float width);
        Element &border_color(const Color &color);
        Element &gap_border(float width, const Color &color);
        Element &gap_border_width(float width);
        Element &gap_border_color(const Color &color);

        Element &floating_offset(float offsetX, float offsetY);
        Element &floating_offset_x(float offsetX);
        Element &floating_offset_y(float offsetY);
        Element &floating_expand(float expandX, float expandY);
        Element &floating_expand_x(float expandX);
        Element &floating_expand_y(float expandY);
        Element &floating_z_index(int zIndex);
        Element &floating_parent(std::string parentId);
        Element &floating_parent(int parentId);
        Element &floating_attach_point(AttachPointType attach);
        Element &floating_attach_point_self(AttachPointType attach);
        Element &floating_attach_point_parent(AttachPointType attach);
        Element &floating_pointer_mode(PointerEventMode pointerMode);

        // @brief Gets the computed properties of an element.
        ComputedProps get_computed_properties();


        // Event handlers
        Element &capture_drag();
        Element &stop_capture_drag();

        /// Sets a hover callback that runs every frame while the mouse is over the element. Takes a generic user data parameter of any type.
        ///
        /// Usage with lambda:
        ///```cpp
        ///     Element("Example").on_hovering(userData,
        ///         [](Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
        ///         {
        ///             // Handle hover state
        ///         });
        ///```
        ///
        /// Usage with external function:
        ///```cpp
        ///     void handle_hovering(Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
        ///     {
        ///         // Handle hover state
        ///     }
        ///
        ///     Element("Example").on_hovering(userData, handle_hovering);
        ///```
        template <typename TUserData, DataCallable<TUserData> Callable>
        Element &on_hovering(TUserData &userData, Callable &&callback);

        /// Sets a hover callback that runs every frame while the mouse is over the element.
        ///
        /// Usage with lambda:
        ///```cpp
        ///     Element("Example").on_hovering(
        ///         [](Element& el, ComputedProps props, UIInputs inputs)
        ///         {
        ///             // Handle hover state
        ///         });
        ///```
        ///
        /// Usage with external function:
        ///```cpp
        ///     void handle_hovering(Element& el, ComputedProps props, UIInputs inputs)
        ///     {
        ///         // Handle hover state
        ///     }
        ///
        ///     Element("Example").on_hovering(handle_hovering);
        ///```
        template <BasicCallable Callable>
        Element &on_hovering(Callable &&callback);

        /// Sets a callback that runs every frame while the mouse button is held down over the element. Takes a generic user data parameter of any type.
        ///
        /// Usage with lambda:
        ///```cpp
        ///     Element("Example").on_pressing(userData,
        ///         [](Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
        ///         {
        ///             // Handle press state
        ///         });
        ///```
        ///
        /// Usage with external function:
        ///```cpp
        ///     void handle_pressing(Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
        ///     {
        ///         // Handle press state
        ///     }
        ///
        ///     Element("Example").on_pressing(userData, handle_pressing);
        ///```
        template <typename TUserData, DataCallable<TUserData> Callable>
        Element &on_pressing(TUserData &userData, Callable &&callback);

        /// Sets a callback that runs every frame while the mouse button is held down over the element.
        ///
        /// Usage with lambda:
        ///```cpp
        ///     Element("Example").on_pressing(
        ///         [](Element& el, ComputedProps props, UIInputs inputs)
        ///         {
        ///             // Handle press state
        ///         });
        ///```
        ///
        /// Usage with external function:
        ///```cpp
        ///     void handle_pressing(Element& el, ComputedProps props, UIInputs inputs)
        ///     {
        ///         // Handle press state
        ///     }
        ///
        ///     Element("Example").on_pressing(handle_pressing);
        ///```
        template <BasicCallable Callable>
        Element &on_pressing(Callable &&callback);

        /// Sets a callback that runs once when the mouse first enters the element's bounds. Takes a generic user data parameter of any type.
        ///
        /// Usage with lambda:
        ///```cpp
        ///     Element("Example").on_hover(userData,
        ///         [](Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
        ///         {
        ///             // Handle hover enter
        ///         });
        ///```
        ///
        /// Usage with external function:
        ///```cpp
        ///     void handle_hover(Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
        ///     {
        ///         // Handle hover enter
        ///     }
        ///
        ///     Element("Example").on_hover(userData, handle_hover);
        ///```
        template <typename TUserData, DataCallable<TUserData> Callable>
        Element &on_hover(TUserData &userData, Callable &&callback);

        /// Sets a callback that runs once when the mouse first enters the element's bounds.
        ///
        /// Usage with lambda:
        ///```cpp
        ///     Element("Example").on_hover(
        ///         [](Element& el, ComputedProps props, UIInputs inputs)
        ///         {
        ///             // Handle hover enter
        ///         });
        ///```
        ///
        /// Usage with external function:
        ///```cpp
        ///     void handle_hover(Element& el, ComputedProps props, UIInputs inputs)
        ///     {
        ///         // Handle hover enter
        ///     }
        ///
        ///     Element("Example").on_hover(handle_hover);
        ///```
        template <BasicCallable Callable>
        Element &on_hover(Callable &&callback);

        /// Sets a callback that runs once when the mouse clicks the element. Takes a generic user data parameter of any type.
        ///
        /// Usage with lambda:
        ///```cpp
        ///     Element("Example").on_click(userData,
        ///         [](Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
        ///         {
        ///             // Handle click
        ///         });
        ///```
        ///
        /// Usage with external function:
        ///```cpp
        ///     void handle_click(Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
        ///     {
        ///         // Handle click
        ///     }
        ///
        ///     Element("Example").on_click(userData, handle_click);
        ///```
        template <typename TUserData, DataCallable<TUserData> Callable>
        Element &on_click(TUserData &userData, Callable &&callback);

        /// Sets a callback that runs once when the mouse clicks the element.
        ///
        /// Usage with lambda:
        ///```cpp
        ///     Element("Example").on_click(
        ///         [](Element& el, ComputedProps props, UIInputs inputs)
        ///         {
        ///             // Handle click
        ///         });
        ///```
        ///
        /// Usage with external function:
        ///```cpp
        ///     void handle_click(Element& el, ComputedProps props, UIInputs inputs)
        ///     {
        ///         // Handle click
        ///     }
        ///
        ///     Element("Example").on_click(handle_click);
        ///```
        template <BasicCallable Callable>
        Element &on_click(Callable &&callback);

        /// Sets a callback that runs each frame when the mouse moves while over the element. Takes a generic user data parameter of any type.
        ///
        /// Usage with lambda:
        ///```cpp
        ///     Element("Example").on_mouse_move(userData,
        ///         [](Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
        ///         {
        ///             // Handle mouse movement
        ///         });
        ///```
        ///
        /// Usage with external function:
        ///```cpp
        ///     void handle_mouse_move(Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
        ///     {
        ///         // Handle mouse movement
        ///     }
        ///
        ///     Element("Example").on_mouse_move(userData, handle_mouse_move);
        ///```
        template <typename TUserData, DataCallable<TUserData> Callable>
        Element &on_mouse_move(TUserData &userData, Callable &&callback);

        /// Sets a callback that runs each frame when the mouse moves while over the element.
        ///
        /// Usage with lambda:
        ///```cpp
        ///     Element("Example").on_mouse_move(
        ///         [](Element& el, ComputedProps props, UIInputs inputs)
        ///         {
        ///             // Handle mouse movement
        ///         });
        ///```
        ///
        /// Usage with external function:
        ///```cpp
        ///     void handle_mouse_move(Element& el, ComputedProps props, UIInputs inputs)
        ///     {
        ///         // Handle mouse movement
        ///     }
        ///
        ///     Element("Example").on_mouse_move(handle_mouse_move);
        ///```
        template <BasicCallable Callable>
        Element &on_mouse_move(Callable &&callback);

        /// Sets a callback that runs each frame when the mouse scrolls while over the element. Takes a generic user data parameter of any type.
        ///
        /// Usage with lambda:
        ///```cpp
        ///     Element("Example").on_scroll(userData,
        ///         [](Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
        ///         {
        ///             // Handle scroll
        ///         });
        ///```
        ///
        /// Usage with external function:
        ///```cpp
        ///     void handle_scroll(Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
        ///     {
        ///         // Handle scroll
        ///     }
        ///
        ///     Element("Example").on_scroll(userData, handle_scroll);
        ///```
        template <typename TUserData, DataCallable<TUserData> Callable>
        Element &on_scroll(TUserData &userData, Callable &&callback);

        /// Sets a callback that runs each frame when the mouse scrolls while over the element.
        ///
        /// Usage with lambda:
        ///```cpp
        ///     Element("Example").on_scroll(
        ///         [](Element& el, ComputedProps props, UIInputs inputs)
        ///         {
        ///             // Handle scroll
        ///         });
        ///```
        ///
        /// Usage with external function:
        ///```cpp
        ///     void handle_scroll(Element& el, ComputedProps props, UIInputs inputs)
        ///     {
        ///         // Handle scroll
        ///     }
        ///
        ///     Element("Example").on_scroll(handle_scroll);
        ///```
        template <BasicCallable Callable>
        Element &on_scroll(Callable &&callback);

        /// Sets a callback that runs each frame when the element is being dragged (clicked and moved). Takes a generic user data parameter of any type.
        ///
        /// Usage with lambda:
        ///```cpp
        ///     Element("Example").on_drag(userData,
        ///         [](Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
        ///         {
        ///             // Handle drag
        ///         });
        ///```
        ///
        /// Usage with external function:
        ///```cpp
        ///     void handle_drag(Element& el, ComputedProps props, UIInputs inputs, UserDataType &userData)
        ///     {
        ///         // Handle drag
        ///     }
        ///
        ///     Element("Example").on_drag(userData, handle_drag);
        ///```
        template <typename TUserData, DataCallable<TUserData> Callable>
        Element &on_drag(TUserData &userData, Callable &&callback);

        /// Sets a callback that runs each frame when the element is being dragged (clicked and moved).
        ///
        /// Usage with lambda:
        ///```cpp
        ///     Element("Example").on_drag(
        ///         [](Element& el, ComputedProps props, UIInputs inputs)
        ///         {
        ///             // Handle drag
        ///         });
        ///```
        ///
        /// Usage with external function:
        ///```cpp
        ///     void handle_drag(Element& el, ComputedProps props, UIInputs inputs)
        ///     {
        ///         // Handle drag
        ///     }
        ///
        ///     Element("Example").on_drag(handle_drag);
        ///```
        template <BasicCallable Callable>
        Element &on_drag(Callable &&callback);

        /// @brief Checks if the current element is hovered by the mouse.
        static bool is_hovered();
        /// @brief Checks if the current element is being pressed by the mouse.
        static bool hovered_this_frame();
        /// @brief Checks if the current element is being pressed by the mouse.
        static bool is_pressed();
        /// @brief Checks if the current element was clicked this frame.
        static bool clicked_this_frame();
        /// @brief Checks if the current element started being hovered this frame.
        static bool mouse_moved_this_frame();
        /// @brief Checks if the mouse was scrolled while over the current element this frame.
        static bool mouse_scrolled_this_frame();
        /// @brief Checks if the mouse was dragged while over the current element this frame.
        static bool mouse_dragged_this_frame();
        /// @brief Get the computed properties of the current element.
        static ComputedProps computed();

        void internal_begin();
        static void internal_end();
};

/// @brief Sets a callback to run every frame the mouse is hovering over the element.
template <BasicCallable Callable>
Element &Element::on_hovering(Callable &&callback)
{
    if (is_hovered())
    {
        callback(*this, computed(), ClayState::inputs);
    }

    return *this;
}

/// @brief Sets a callback to run every frame the mouse is hovering over the element. Takes a generic user data parameter.
template <typename TUserData, DataCallable<TUserData> Callable>
Element &Element::on_hovering(TUserData &userData, Callable &&callback)
{
    if (is_hovered())
    {
        callback(*this, computed(), ClayState::inputs, userData);
    }

    return *this;
}

/// @brief Sets a callback to run every frame the mouse is pressing the element.
template <BasicCallable Callable>
Element &Element::on_pressing(Callable &&callback)
{
    if (is_pressed())
    {
        callback(*this, computed(), ClayState::inputs);
    }

    return *this;
}

/// @brief Sets a callback to run every frame the mouse is pressing the element.
template <typename TUserData, DataCallable<TUserData> Callable>
Element &Element::on_pressing(TUserData &userData, Callable &&callback)
{
    if (is_pressed())
    {
        callback(*this, computed(), ClayState::inputs, userData);
    }

    return *this;
}

/// @brief Sets a callback to run once on the frame the mouse enters the element's bounds.
template <BasicCallable Callable>
Element &Element::on_hover(Callable &&callback)
{
    if (hovered_this_frame())
    {
        callback(*this, computed(), ClayState::inputs);
    }

    return *this;
}

/// @brief Sets a callback to run once on the frame the mouse enters the element's bounds. Takes a generic user data parameter.
template <typename TUserData, DataCallable<TUserData> Callable>
Element &Element::on_hover(TUserData &userData, Callable &&callback)
{
    if (hovered_this_frame())
    {
        callback(*this, computed(), ClayState::inputs, userData);
    }

    return *this;
}

/// @brief Sets a callback to run once on the frame the mouse clicks the element.
template <BasicCallable Callable>
Element &Element::on_click(Callable &&callback)
{
    if (clicked_this_frame())
    {
        callback(*this, computed(), ClayState::inputs);
    }

    return *this;
}

/// @brief Sets a callback to run once on the frame the mouse clicks the element. Takes a generic user data parameter.
template <typename TUserData, DataCallable<TUserData> Callable>
Element &Element::on_click(TUserData &userData, Callable &&callback)
{
    if (clicked_this_frame())
    {
        callback(*this, computed(), ClayState::inputs, userData);
    }

    return *this;
}

/// @brief Sets a callback to run each frame the mouse moves while over the element.
template <BasicCallable Callable>
Element &Element::on_mouse_move(Callable &&callback)
{
    if (mouse_moved_this_frame())
    {
        callback(*this, computed(), ClayState::inputs);
    }

    return *this;
}

/// @brief Sets a callback to run each frame the mouse moves while over the element.
template <typename TUserData, DataCallable<TUserData> Callable>
Element &Element::on_mouse_move(TUserData &userData, Callable &&callback)
{
    if (mouse_moved_this_frame())
    {
        callback(*this, computed(), ClayState::inputs, userData);
    }

    return *this;
}

/// @brief Sets a callback to run each frame the mouse scrolls while over the element.
template <BasicCallable Callable>
Element &Element::on_scroll(Callable &&callback)
{
    if (mouse_scrolled_this_frame())
    {
        callback(*this, computed(), ClayState::inputs);
    }

    return *this;
}

/// @brief Sets a callback to run each frame the mouse scrolls while over the element.
template <typename TUserData, DataCallable<TUserData> Callable>
Element &Element::on_scroll(TUserData &userData, Callable &&callback)
{
    if (mouse_scrolled_this_frame())
    {
        callback(*this, computed(), ClayState::inputs, userData);
    }

    return *this;
}

/// @brief Sets a callback to run each frame the element is clicked and dragged.
template <BasicCallable Callable>
Element &Element::on_drag(Callable &&callback)
{
    if (mouse_dragged_this_frame())
    {
        callback(*this, computed(), ClayState::inputs);
    }

    return *this;
}

/// @brief Sets a callback to run each frame the element is clicked and dragged.
template <typename TUserData, DataCallable<TUserData> Callable>
Element &Element::on_drag(TUserData &userData, Callable &&callback)
{
    if (mouse_dragged_this_frame())
    {
        callback(*this, computed(), ClayState::inputs, userData);
    }

    return *this;
}