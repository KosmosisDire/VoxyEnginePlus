#pragma once

#include "window.hpp"
#include <chrono>
#include <cmath>
#include <fonts.hpp>
#include <input.hpp>
#include <iostream>
#include <memory>
#include <renderer.hpp>
#include <stdio.h>
#include <thread>

using namespace std::chrono_literals;
using Clock = std::chrono::high_resolution_clock;

struct Application
{
  public:
    Application(std::string name, std::vector<std::filesystem::path> shaderDirectories)
        : window(std::make_shared<GameWindow>(name, 1920, 1080)), inputManager(window), renderer(std::make_shared<Renderer>(window, shaderDirectories))
    {
        start = Clock::now();
        window->SetResizeCallback([this](u32 sx, u32 sy)
                                  { OnResizeBase(sx, sy); });
    }
    virtual ~Application() {};

    // Prevent copying
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    bool Update()
    {
        if (!initialized)
        {
            initialized = true;
            renderer->Resize(window->GetWidth(), window->GetHeight());
            OnResize(window->GetWidth(), window->GetHeight());
            OnStart();
        }

        Input::Update();
        window->Update();

        if (window->ShouldClose())
            return true;

        if (window->IsMinimized())
        {
            std::this_thread::sleep_for(1ms);
            return false;
        }

        CalcDeltatime();

        OnUpdate(delta_time);

        renderer->Render();

        renderer->device.collect_garbage();

        return false;
    }

    float GetTime() const { return time; }

  protected:
    std::shared_ptr<GameWindow> window;
    std::shared_ptr<Renderer> renderer;

    virtual void OnResize(u32 sx, u32 sy) {};
    virtual void OnUpdate(float dt) {};
    virtual void OnStart() {};

  private:
    Input inputManager;
    Clock::time_point start, prev_time = Clock::now();
    f32 time = 0.0f;
    f32 delta_time = 1.0f;
    bool initialized = false;

    void OnResizeBase(u32 sx, u32 sy)
    {
        if (window->IsMinimized())
            return;

        renderer->Resize(sx, sy);
        OnResize(sx, sy);
        Update();
    }

    void CalcDeltatime()
    {
        auto now = Clock::now();
        time = std::chrono::duration<f32>(now - start).count();
        delta_time = std::chrono::duration<f32>(now - prev_time).count();
        prev_time = now;
    }
};