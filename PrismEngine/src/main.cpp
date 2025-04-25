#include <project_paths.h>
#include "voxel-app.hpp"
#include <engine/apis/Resources.hpp>

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
