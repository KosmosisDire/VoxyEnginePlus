#pragma once

#include <daxa/daxa.hpp>
#include <functional>
#include <iostream>

using namespace daxa::types;

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_NATIVE_INCLUDE_NONE
using HWND = void *;
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
// #define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#include <GLFW/glfw3native.h>

class GameWindow
{
    public:
        using ResizeCallback = std::function<void(u32, u32)>;

        GameWindow(std::string window_name, u32 sx, u32 sy)
            : size_x{sx}, size_y{sy}, window_name{window_name}
        {
            if (!glfwInit())
            {
                std::cerr << "Failed to initialize GLFW" << std::endl;
                return;
            }

            // Set platform-specific hints before window creation
#if defined(__linux__)
            // Set up window for Wayland or X11
            check_and_set_platform_hints();
#endif

            // Common window hints
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
            glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE); // For HiDPI support

            glfwWindowPointer = glfwCreateWindow(
                                    static_cast<i32>(size_x),
                                    static_cast<i32>(size_y),
                                    window_name.c_str(),
                                    nullptr,
                                    nullptr);

            if (!glfwWindowPointer)
            {
                std::cerr << "Failed to create GLFW window" << std::endl;
                glfwTerminate();
                return;
            }

            // Log which platform we're actually using
            log_platform_info();

            glfwSetWindowUserPointer(glfwWindowPointer, this);

            glfwSetFramebufferSizeCallback(glfwWindowPointer,
                                           [](GLFWwindow * window, i32 sx, i32 sy)
            {
                auto self = reinterpret_cast<GameWindow*>(glfwGetWindowUserPointer(window));
                self->size_x = static_cast<u32>(sx);
                self->size_y = static_cast<u32>(sy);
                self->minimized = sx == 0 || sy == 0;

                if (self->resize_callback)
                {
                    self->resize_callback(self->size_x, self->size_y);
                }
            });
        }

        ~GameWindow()
        {
            if (glfwWindowPointer)
            {
                glfwDestroyWindow(glfwWindowPointer);
            }

            glfwTerminate();
        }

        // Prevent copying
        GameWindow(const GameWindow &) = delete;
        GameWindow &operator=(const GameWindow &) = delete;

        inline daxa::NativeWindowHandle get_native_handle()
        {
            if (!glfwWindowPointer)
            {
                std::cerr << "ERROR: Window pointer is null!" << std::endl;
                return nullptr;
            }

            int platform = glfwGetPlatform();
            std::cout << "Current platform: " << platform << std::endl;

#if defined(_WIN32)
            return glfwGetWin32Window(glfwWindowPointer);
#elif defined(__linux__)

            // if (platform == GLFW_PLATFORM_WAYLAND)
            // {
            //     void* wayland_handle = glfwGetWaylandWindow(glfwWindowPointer);
            //     std::cout << "Wayland window handle: " << wayland_handle << std::endl;
            //     return reinterpret_cast<daxa::NativeWindowHandle>(wayland_handle);
            // }
            if (platform == GLFW_PLATFORM_X11 || platform == GLFW_PLATFORM_WAYLAND)
            {
                Window x11_handle = glfwGetX11Window(glfwWindowPointer);
                std::cout << "X11 window handle: " << x11_handle << std::endl;
                return reinterpret_cast<daxa::NativeWindowHandle>(reinterpret_cast<void*>(static_cast<uintptr_t>(x11_handle)));
            }
            else
            {
                std::cerr << "Unknown platform: " << platform << std::endl;
                return nullptr;
            }

#endif
        }

        inline daxa::NativeWindowPlatform get_native_platform()
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

        GLFWwindow *get_glfw_window() const { return glfwWindowPointer; }

        u32 get_width() const { return size_x; }

        u32 get_height() const { return size_y; }

        bool is_minimized() const { return minimized; }

        void set_resize_callback(ResizeCallback callback) { resize_callback = callback; }

        bool should_close()
        {
            return glfwWindowPointer && glfwWindowShouldClose(glfwWindowPointer);
        }

        void update()
        {
            if (glfwWindowPointer)
            {
                glfwPollEvents();
                // Note: glfwSwapBuffers is not necessary with GLFW_NO_API
                // as you're managing the presentation yourself with your graphics API
            }
        }

    private:
        void check_and_set_platform_hints()
        {
#if defined(__linux__)
            // Check environment variables to determine which platform to use
            const char* wayland_display = getenv("WAYLAND_DISPLAY");
            const char* display = getenv("DISPLAY");

            if (wayland_display)
            {
                // Prefer Wayland if available
                glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
                std::cout << "Preferring Wayland platform" << std::endl;
            }
            else if (display)
            {
                // Fall back to X11 if Wayland is not available
                glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
                std::cout << "Preferring X11 platform" << std::endl;
            }

#endif
        }

        void log_platform_info()
        {
            if (!glfwWindowPointer)
            {
                return;
            }

            int platform = glfwGetPlatform();
            std::cout << "GLFW Platform: ";

            switch (platform)
            {
                case GLFW_PLATFORM_WIN32:
                    std::cout << "Windows";
                    break;

                case GLFW_PLATFORM_X11:
                    std::cout << "X11";
                    break;

                case GLFW_PLATFORM_WAYLAND:
                    std::cout << "Wayland";
                    break;

                default:
                    std::cout << "Unknown (" << platform << ")";
                    break;
            }

            std::cout << std::endl;
        }

        GLFWwindow *glfwWindowPointer = nullptr;
        u32 size_x, size_y;
        bool minimized = false;
        ResizeCallback resize_callback;
        std::string window_name;
};
