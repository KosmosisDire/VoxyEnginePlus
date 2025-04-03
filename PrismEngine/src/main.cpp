#include "project_paths.h"
#include "voxel-app.hpp"
#include <engine/apis/Resources.hpp>
#include <scripting/scripting.hpp>

auto main() -> int
{
    scriptingEngine.registerStdLib();
    scriptingEngine.registerEngineAPI();

    VoxyApp app;

    scripts->addScriptFile(Resources::GetResourcePath("test.as").string());
    scriptingEngine.savePredefinedScript(PROJECT_ROOT_DIR "/as.predefined");

    while (true)
    {
        scriptingEngine.checkHotReload();

        if (app.Update())
        {
            break;
        }
    }

    return 0;
}