#include "voxel-app.hpp"

auto main() -> int
{
    VoxyApp app;
    // GameWindow window = GameWindow("App", 1920, 1080);
    while (true)
    {
        // window.Update();
        if (app.Update())
        {
            break;
        }
    }
    return 0;
}