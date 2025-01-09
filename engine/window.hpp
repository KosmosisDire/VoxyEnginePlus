#pragma once

#include <daxa/daxa.hpp>
#include <functional>

using namespace daxa::types;

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_NATIVE_INCLUDE_NONE
using HWND = void *;
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#include <GLFW/glfw3native.h>

class Window
{
  public:
    using MouseMoveCallback = std::function<void(f32, f32)>;
    using MouseButtonCallback = std::function<void(i32, i32)>;
    using KeyCallback = std::function<void(i32, i32)>;
    using ResizeCallback = std::function<void(u32, u32)>;

    explicit Window(char const *window_name, u32 sx = 1920, u32 sy = 1080);
    ~Window();

    // Prevent copying
    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    auto get_native_handle() -> daxa::NativeWindowHandle;
    auto get_native_platform() -> daxa::NativeWindowPlatform;

    void set_mouse_pos(f32 x, f32 y);
    void set_mouse_capture(bool should_capture);

    void set_mouse_move_callback(MouseMoveCallback callback)
    {
        mouse_move_callback = std::move(callback);
    }
    void set_mouse_button_callback(MouseButtonCallback callback)
    {
        mouse_button_callback = std::move(callback);
    }
    void set_key_callback(KeyCallback callback)
    {
        key_callback = std::move(callback);
    }
    void set_resize_callback(ResizeCallback callback)
    {
        resize_callback = std::move(callback);
    }

    GLFWwindow *get_glfw_window() const { return glfw_window_ptr; }
    u32 get_width() const { return size_x; }
    u32 get_height() const { return size_y; }
    bool is_minimized() const { return minimized; }
    void set_minimized(bool min) { minimized = min; }

  private:
    GLFWwindow *glfw_window_ptr;
    u32 size_x, size_y;
    bool minimized = false;

    MouseMoveCallback mouse_move_callback;
    MouseButtonCallback mouse_button_callback;
    KeyCallback key_callback;
    ResizeCallback resize_callback;

    static void mouse_move_callback_wrapper(GLFWwindow *window, f64 x, f64 y);
    static void mouse_button_callback_wrapper(GLFWwindow *window, i32 button,
                                              i32 action, i32 mods);
    static void key_callback_wrapper(GLFWwindow *window, i32 key, i32 scancode,
                                     i32 action, i32 mods);
    static void resize_callback_wrapper(GLFWwindow *window, i32 width,
                                        i32 height);
};

inline Window::Window(char const *window_name, u32 sx, u32 sy)
    : size_x{sx}, size_y{sy}
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfw_window_ptr =
        glfwCreateWindow(static_cast<i32>(size_x), static_cast<i32>(size_y),
                         window_name, nullptr, nullptr);
    glfwSetWindowUserPointer(glfw_window_ptr, this);

    glfwSetCursorPosCallback(glfw_window_ptr, mouse_move_callback_wrapper);
    glfwSetMouseButtonCallback(glfw_window_ptr, mouse_button_callback_wrapper);
    glfwSetKeyCallback(glfw_window_ptr, key_callback_wrapper);
    glfwSetFramebufferSizeCallback(glfw_window_ptr, resize_callback_wrapper);
}

inline Window::~Window()
{
    glfwDestroyWindow(glfw_window_ptr);
    glfwTerminate();
}

inline auto Window::get_native_handle() -> daxa::NativeWindowHandle
{
#if defined(_WIN32)
    return glfwGetWin32Window(glfw_window_ptr);
#elif defined(__linux__)
    switch (get_native_platform())
    {
    case daxa::NativeWindowPlatform::WAYLAND_API:
        return reinterpret_cast<daxa::NativeWindowHandle>(
            glfwGetWaylandWindow(glfw_window_ptr));
    case daxa::NativeWindowPlatform::XLIB_API:
    default:
        return reinterpret_cast<daxa::NativeWindowHandle>(
            glfwGetX11Window(glfw_window_ptr));
    }
#endif
}

inline auto Window::get_native_platform() -> daxa::NativeWindowPlatform
{
    switch (glfwGetPlatform())
    {
    case GLFW_PLATFORM_WIN32:
        return daxa::NativeWindowPlatform::WIN32_API;
    case GLFW_PLATFORM_X11:
        return daxa::NativeWindowPlatform::XLIB_API;
    case GLFW_PLATFORM_WAYLAND:
        return daxa::NativeWindowPlatform::WAYLAND_API;
    default:
        return daxa::NativeWindowPlatform::UNKNOWN;
    }
}

inline void Window::set_mouse_pos(f32 x, f32 y)
{
    glfwSetCursorPos(glfw_window_ptr, static_cast<f64>(x), static_cast<f64>(y));
}

inline void Window::set_mouse_capture(bool should_capture)
{
    glfwSetCursorPos(glfw_window_ptr, static_cast<f64>(size_x / 2),
                     static_cast<f64>(size_y / 2));
    glfwSetInputMode(glfw_window_ptr, GLFW_CURSOR,
                     should_capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwSetInputMode(glfw_window_ptr, GLFW_RAW_MOUSE_MOTION, should_capture);
}

inline void Window::mouse_move_callback_wrapper(GLFWwindow *window, f64 x,
                                                f64 y)
{
    auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (win && win->mouse_move_callback)
    {
        win->mouse_move_callback(static_cast<f32>(x), static_cast<f32>(y));
    }
}

inline void Window::mouse_button_callback_wrapper(GLFWwindow *window,
                                                  i32 button, i32 action,
                                                  i32 mods)
{
    auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (win && win->mouse_button_callback)
    {
        win->mouse_button_callback(button, action);
    }
}

inline void Window::key_callback_wrapper(GLFWwindow *window, i32 key,
                                         i32 scancode, i32 action, i32 mods)
{
    auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (win && win->key_callback)
    {
        win->key_callback(key, action);
    }
}

inline void Window::resize_callback_wrapper(GLFWwindow *window, i32 width,
                                            i32 height)
{
    auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (win && win->resize_callback)
    {
        win->resize_callback(static_cast<u32>(width), static_cast<u32>(height));
    }
}