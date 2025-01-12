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
    using ResizeCallback = std::function<void(u32, u32)>;

    Window(std::string window_name, u32 sx, u32 sy)
        : size_x{sx}, size_y{sy}
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        glfwWindowPointer = glfwCreateWindow(static_cast<i32>(size_x), static_cast<i32>(size_y), window_name.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(glfwWindowPointer, this);

        glfwSetFramebufferSizeCallback(glfwWindowPointer, [](GLFWwindow *window, i32 sx, i32 sy) {
            auto self = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
            self->size_x = static_cast<u32>(sx);
            self->size_y = static_cast<u32>(sy);
            if (self->resize_callback)
                self->resize_callback(self->size_x, self->size_y);
            self->minimized = sx == 0 || sy == 0;
        });
    }

    ~Window()
    {
        glfwDestroyWindow(glfwWindowPointer);
        glfwTerminate();
    }

    // Prevent copying
    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    inline daxa::NativeWindowHandle GetNativeHandle()
    {
#if defined(_WIN32)
        return glfwGetWin32Window(glfwWindowPointer);
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

    inline daxa::NativeWindowPlatform GetNativePlatform()
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

    GLFWwindow *GetGlfwWindow() const { return glfwWindowPointer; }
    u32 GetWidth() const { return size_x; }
    u32 GetHeight() const { return size_y; }
    bool IsMinimized() const { return minimized; }
    void SetResizeCallback(ResizeCallback callback) { resize_callback = callback; }
    bool ShouldClose() { return glfwWindowShouldClose(glfwWindowPointer); }
    void Update()
    {
        glfwPollEvents();
        glfwSwapBuffers(glfwWindowPointer);
    }

  private:
    GLFWwindow *glfwWindowPointer;
    u32 size_x, size_y;
    bool minimized = false;
    ResizeCallback resize_callback;
};
