#pragma once

#include "scripting-engine.hpp"

void ErrorHandler(ScriptingEngine::LogLevel level, const std::string &message)
{
    switch (level)
    {
        case ScriptingEngine::LogLevel::Debug:
            std::cout << "[DEBUG] " << message << std::endl;
            break;

        case ScriptingEngine::LogLevel::Info:
            std::cout << "[INFO] " << message << std::endl;
            break;

        case ScriptingEngine::LogLevel::Warning:
            std::cout << "[WARNING] " << message << std::endl;
            break;

        case ScriptingEngine::LogLevel::Error:
            std::cerr << "[ERROR] " << message << std::endl;
            break;

        case ScriptingEngine::LogLevel::Critical:
            std::cerr << "[CRITICAL] " << message << std::endl;
            break;
    }
}

ScriptingEngine scriptingEngine(ScriptingEngine::LogLevel::Debug, ErrorHandler);
std::shared_ptr<ScriptingEngine::ScriptModule> scripts = scriptingEngine.getModule("main");
