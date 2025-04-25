#pragma once

#include "Window.hpp"
#include <chrono>
#include <engine/apis/Fonts.hpp>
#include <engine/apis/Input.hpp>
#include <memory>
#include <engine/objects/Renderer.hpp>
#include <thread>

using namespace std::chrono_literals;
using Clock = std::chrono::high_resolution_clock;

class Application
{
    public:
        Application(std::string name, std::vector<std::filesystem::path> shaderDirectories = {"resources/shaders"})
            : window(std::make_shared<GameWindow>(name, 1920, 1080)), inputManager(window), renderer(std::make_shared<Renderer>(window, shaderDirectories))
        {
            start = Clock::now();
            window->set_resize_callback([this](u32 sx, u32 sy) // Update call
            { on_resize_base(sx, sy); }); // Update call
        }

        virtual ~Application() {};

        // Prevent copying
        Application(const Application &) = delete;
        Application &operator=(const Application &) = delete;

        bool update()
        {
            if (!initialized)
            {
                initialized = true;
                renderer->resize(window->get_width(), window->get_height()); // Update calls
                on_resize(window->get_width(), window->get_height()); // Update calls
                on_start(); // Update call
            }

            Input::update();
            window->update(); // Update call

            if (window->should_close()) // Update call
            {
                return true;
            }

            if (window->is_minimized()) // Update call
            {
                std::this_thread::sleep_for(1ms);
                return false;
            }

            calc_deltatime(); // Update call

            on_update(delta_time); // Update call

            renderer->render(); // Update call

            renderer->device.collect_garbage();

            return false;
        }

        float get_time() const { return time; }

    protected:
        std::shared_ptr<GameWindow> window;
        std::shared_ptr<Renderer> renderer;

        virtual void on_resize(u32 sx, u32 sy) {};

        virtual void on_update(float dt) {};

        virtual void on_start() {};

    private:
        Input inputManager;
        Clock::time_point start, prev_time = Clock::now();
        f32 time = 0.0f;
        f32 delta_time = 1.0f;
        bool initialized = false;

        void on_resize_base(u32 sx, u32 sy)
        {
            if (window->is_minimized()) // Update call
            {
                return;

            }

            renderer->resize(sx, sy); // Update call
            on_resize(sx, sy); // Update call
            update(); // Update call
        }

        void calc_deltatime()
        {
            auto now = Clock::now();
            time = std::chrono::duration<f32>(now - start).count();
            delta_time = std::chrono::duration<f32>(now - prev_time).count();
            prev_time = now;
        }
};
