#include "voxel-app.hpp"

auto main() -> int
{
    VoxyApp app;
    while (true)
    {
        if (app.update())
        {
            break;
        }
    }
    return 0;
}