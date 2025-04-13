#include "project_paths.h"
#include "voxel-app.hpp"
#include <engine/apis/Resources.hpp>
#include <scripting/scripting.hpp>
#include <scripting/parser_interface.hpp> // Include the new parser interface
#include <iostream> // For console output
#include <filesystem> // For file system operations (hot reloading)
#include <chrono>     // For time points (hot reloading)
#include <fstream>    // For file writing

namespace fs = std::filesystem;

auto main() -> int
{
    // --- Initial Script Parsing ---
    std::string scriptPathStr = Resources::GetResourcePath("test.flow").string();
    fs::path scriptPath(scriptPathStr);
    std::string outputFilePathStr = scriptPathStr + ".c_api.txt"; // Define output file path
    std::string testScript = Resources::ReadAsString("test.flow");

    // --- Generate C-API from script and write to file ---
    std::cout << "--- Generating C-API for: " << scriptPathStr << " ---" << std::endl;
    std::string cApiOutput = Mycelium::Scripting::ParserInterface::generateCApi(testScript);

    try
    {
        std::ofstream outFile(outputFilePathStr);

        if (outFile.is_open())
        {
            outFile << cApiOutput;
            outFile.close();
            std::cout << "--- C-API output written to: " << outputFilePathStr << " ---" << std::endl;
        }
        else
        {
            std::cerr << "Error: Could not open output file: " << outputFilePathStr << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error writing C-API output file: " << e.what() << std::endl;
    }

    // ----------------------------------------------------
    auto lastWriteTime = fs::last_write_time(scriptPath);
    // -----------------------------

    scriptingEngine.registerStdLib();
    scriptingEngine.registerEngineAPI();

    VoxyApp app;

    scripts->addScriptFile(Resources::GetResourcePath("test.as").string());
    scriptingEngine.savePredefinedScript(PROJECT_ROOT_DIR "/as.predefined");

    while (true)
    {
        // --- Hot Reload Check for ANTLR Script ---
        try
        {
            auto currentWriteTime = fs::last_write_time(scriptPath);

            if (currentWriteTime > lastWriteTime)
            {
                std::cout << "Detected change in " << scriptPathStr << ". Reloading and generating C-API..." << std::endl;
                testScript = Resources::ReadAsString("test.flow"); // Re-read the file

                // --- Generate C-API from reloaded script and write to file ---
                std::cout << "--- Generating C-API (Hot Reload) ---" << std::endl;
                cApiOutput = Mycelium::Scripting::ParserInterface::generateCApi(testScript); // Re-generate

                try
                {
                    std::ofstream outFile(outputFilePathStr);

                    if (outFile.is_open())
                    {
                        outFile << cApiOutput;
                        outFile.close();
                        std::cout << "--- C-API output written to: " << outputFilePathStr << " ---" << std::endl;
                    }
                    else
                    {
                        std::cerr << "Error: Could not open output file: " << outputFilePathStr << std::endl;
                    }
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error writing C-API output file (Hot Reload): " << e.what() << std::endl;
                }

                // -----------------------------------------------------------
                lastWriteTime = currentWriteTime; // Update the last write time
                std::cout << "Hot reload processing complete." << std::endl;
                // Optionally: Trigger UI update or other actions based on the new cApiOutput
            }
        }
        catch (const fs::filesystem_error& e)
        {
            // Handle potential errors like file not found, though it should exist
            std::cerr << "Error checking file time: " << e.what() << std::endl;
            // Maybe add a delay before retrying or handle differently
        }

        // -----------------------------------------

        scriptingEngine.checkHotReload(); // Existing hot reload check (assuming for AngelScript?)

        if (app.Update())
        {
            break;
        }
    }

    return 0;
}
