#include "voxel-app.hpp"
#include <scripting/example.hpp>

auto main() -> int
{
    VoxyApp app;
    while (true)
    {
        if (app.Update())
        {
            break;
        }
    }

    return 0;
}