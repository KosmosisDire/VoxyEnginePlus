#include "engine.hpp" // Include our refactored engine header
#include <iostream>
#include <resources.hpp>
#include <fstream>

// Example function to be called from script
void PrintMessage(const std::string& message)
{
    std::cout << message << std::flush;
}

// Custom error callback function
void ErrorHandler(AngelScriptEngine::LogLevel level, const std::string& message) 
{
    switch (level) {
        case AngelScriptEngine::LogLevel::Debug:
            std::cout << "[DEBUG] " << message << std::endl;
            break;
        case AngelScriptEngine::LogLevel::Info:
            std::cout << "[INFO] " << message << std::endl;
            break;
        case AngelScriptEngine::LogLevel::Warning:
            std::cout << "[WARNING] " << message << std::endl;
            break;
        case AngelScriptEngine::LogLevel::Error:
            std::cerr << "[ERROR] " << message << std::endl;
            break;
        case AngelScriptEngine::LogLevel::Critical:
            std::cerr << "[CRITICAL] " << message << std::endl;
            break;
    }
}

// Helper function to create a test script file
bool WriteScriptFile(const std::string& filename, const std::string& content)
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to create script file: " << filename << std::endl;
        return false;
    }
    
    file << content;
    file.close();
    return true;
}

int run()
{
    try
    {
        auto path = Resources::GetResourcePath("test.as");    

        // Create the AngelScript engine
        AngelScriptEngine engine;
        
        // Configure error handling
        engine.setLogLevel(AngelScriptEngine::LogLevel::Debug); // Show all messages
        engine.setErrorCallback(ErrorHandler);
        engine.registerStdLib();

        engine.registerFunction("void print(const string &in)", asFUNCTION(PrintMessage));

        auto module = engine.getModule("Main");
        module->addScriptFile(path.string());
        module->build();

        module->executeFunction("void main()");

        while (true)
        {
            if (engine.checkHotReload())
            {
                module->executeFunction("void main()");
            }
        }

        std::cout << "\nScript processing complete.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}