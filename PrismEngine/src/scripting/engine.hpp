#pragma once

#include <angelscript.h>
#include "addon/scriptstdstring/scriptstdstring.h"
#include "addon/scriptarray/scriptarray.h"
#include "addon/scriptdictionary/scriptdictionary.h"
#include "addon/scriptany/scriptany.h"
#include "addon/scriptmath/scriptmath.h"
#include "addon/scripthelper/scripthelper.h"
#include "addon/weakref/weakref.h"
#include "addon/scripthandle/scripthandle.h"
#include "addon/scriptfile/scriptfile.h"
#include "addon/scriptfile/scriptfilesystem.h"
#include "addon/scriptbuilder/scriptbuilder.h"

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <filesystem>
#include <memory>
#include <chrono>
#include <iostream>
#include <fstream>
#include <optional>
#include <queue>
#include <mutex>


// Callback function type definitions
using ScriptMessageCallback = std::function<void(const asSMessageInfo*, void*)>;
using ScriptFunctionCallback = std::function<void(asIScriptContext*)>;

/**
 * @class AngelScriptEngine
 * @brief A simplified AngelScript engine that handles script loading, hot reloading, 
 *        and provides a flexible interface for exposing C++ functions, variables, and classes.
 */
class AngelScriptEngine {
public:
    // Result struct to improve error handling
    struct Result {
        bool success;
        std::string message;
        
        static Result Ok() {
            return { true, "" };
        }
        
        static Result Error(const std::string& errorMsg) {
            return { false, errorMsg };
        }
        
        operator bool() const { return success; }
    };

    // Error handling and logging
    enum class LogLevel {
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

private:
    LogLevel minLogLevel = LogLevel::Warning; // Default logging level
    std::string lastErrorMessage;
    bool throwOnCriticalError = false;
    std::function<void(LogLevel, const std::string&)> errorCallback = nullptr;

public:
    /**
     * Set minimum log level
     */
    void setLogLevel(LogLevel level) {
        minLogLevel = level;
    }

    /**
     * Set error callback function
     */
    void setErrorCallback(std::function<void(LogLevel, const std::string&)> callback) {
        errorCallback = callback;
    }

    /**
     * Set whether to throw exceptions on critical errors
     */
    void setThrowOnCriticalError(bool shouldThrow) {
        throwOnCriticalError = shouldThrow;
    }

    /**
     * Get the last error message
     */
    std::string getLastError() const {
        return lastErrorMessage;
    }

    /**
     * Log a message with the specified level
     */
    void log(LogLevel level, const std::string& message) {
        // Update last error if this is an error message
        if (level >= LogLevel::Error) {
            lastErrorMessage = message;
        }

        // Skip if below minimum log level
        if (level < minLogLevel) {
            return;
        }

        // Call error callback if registered
        if (errorCallback) {
            errorCallback(level, message);
            return;
        }

        // Default logging to console
        const char* prefix = "";
        switch (level) {
            case LogLevel::Debug:   prefix = "[DEBUG] "; break;
            case LogLevel::Info:    prefix = "[INFO] "; break;
            case LogLevel::Warning: prefix = "[WARNING] "; break;
            case LogLevel::Error:   prefix = "[ERROR] "; break;
            case LogLevel::Critical: prefix = "[CRITICAL] "; break;
        }
        
        std::cerr << prefix << message << std::endl;
        
        // Throw if critical and throwOnCriticalError is true
        if (level == LogLevel::Critical && throwOnCriticalError) {
            throw std::runtime_error(message);
        }
    }

    /**
     * @struct ScriptContext
     * @brief Wraps an AngelScript context with execution functionality
     */
    struct ScriptContext {
        asIScriptContext* context = nullptr;
        AngelScriptEngine* engine = nullptr;

        /**
         * Constructor
         */
        ScriptContext(AngelScriptEngine* engine, asIScriptContext* ctx) 
            : engine(engine), context(ctx) {}

        /**
         * Destructor
         */
        ~ScriptContext() {
            if (context) {
                // Return to pool if engine still exists, otherwise release
                if (engine) {
                    engine->returnContextToPool(context);
                } else {
                    context->Release();
                }
                context = nullptr;
            }
        }

        /**
         * Check if context is valid
         */
        bool isValid() const {
            return context != nullptr;
        }

        /**
         * Prepare the context for execution with a function
         */
        Result prepare(asIScriptFunction* func) {
            if (!context || !func) {
                return Result::Error("Invalid context or function");
            }
            
            int r = context->Prepare(func);
            if (r < 0) {
                return Result::Error("Failed to prepare context for function: " + 
                    std::string(func->GetDeclaration()) + " (error code: " + std::to_string(r) + ")");
            }
            
            return Result::Ok();
        }

        /**
         * Execute a function with this context
         */
        Result execute(asIScriptFunction* func, 
                     const std::vector<ScriptFunctionCallback>& argCallbacks = {},
                     std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
            if (!context || !func) {
                return Result::Error("Invalid context or function");
            }
            
            // Use default timeout if not specified
            if (timeout.count() == 0) {
                timeout = engine->defaultScriptTimeout;
            }

            // Prepare the context
            auto result = prepare(func);
            if (!result) {
                return result;
            }

            // Set arguments if provided
            for (size_t i = 0; i < argCallbacks.size() && i < func->GetParamCount(); ++i) {
                if (argCallbacks[i]) {
                    argCallbacks[i](context);
                }
            }

            // Set up timeout for this specific execution
            std::pair<std::chrono::steady_clock::time_point, std::chrono::milliseconds> timeoutInfo{
                std::chrono::steady_clock::now(), timeout
            };
            context->SetLineCallback(asFUNCTION(LineCallbackFunction), &timeoutInfo, asCALL_CDECL);

            // Execute the function
            int r = context->Execute();

            // Handle execution result
            if (r == asEXECUTION_FINISHED) {
                return Result::Ok();
            }
            else if (r == asEXECUTION_EXCEPTION) {
                std::string errorMsg = "Script exception: " + std::string(context->GetExceptionString()) + 
                    " in function " + context->GetExceptionFunction()->GetDeclaration() + 
                    " at line " + std::to_string(context->GetExceptionLineNumber());
                return Result::Error(errorMsg);
            }
            else if (r == asEXECUTION_ABORTED) {
                return Result::Error("Script execution was aborted (timeout after " + 
                    std::to_string(timeout.count()) + " ms)");
            }
            else {
                return Result::Error("Script execution error: " + std::to_string(r));
            }
        }

        /**
         * Get return value as an int
         */
        int getReturnInt() const {
            return context ? context->GetReturnDWord() : 0;
        }

        /**
         * Get return value as a float
         */
        float getReturnFloat() const {
            return context ? context->GetReturnFloat() : 0.0f;
        }

        /**
         * Get return value as a double
         */
        double getReturnDouble() const {
            return context ? context->GetReturnDouble() : 0.0;
        }

        /**
         * Get return value as a bool
         */
        bool getReturnBool() const {
            return context ? context->GetReturnByte() != 0 : false;
        }

        /**
         * Get return value as a string
         */
        std::string getReturnString() const {
            if (!context) return "";
            void* ptr = context->GetReturnObject();
            if (!ptr) return "";
            return *reinterpret_cast<std::string*>(ptr);
        }

    private:
        /**
         * Line callback function for timeout handling
         */
        static void LineCallbackFunction(asIScriptContext* ctx, void* param) {
            auto* timeoutInfo = static_cast<std::pair<std::chrono::steady_clock::time_point, std::chrono::milliseconds>*>(param);
            auto currentTime = std::chrono::steady_clock::now();
            
            if (currentTime > timeoutInfo->first + timeoutInfo->second) {
                ctx->Abort();
            }
        }
    };

    /**
     * @struct ScriptModule
     * @brief Represents a module within the AngelScript engine with its own functionality
     */
    struct ScriptModule {
        std::string name;                    // Module name
        asIScriptModule* module = nullptr;   // AngelScript module
        std::shared_ptr<CScriptBuilder> builder = nullptr; // Script builder for this module
        std::map<std::string, std::filesystem::file_time_type> scriptFiles; // Script files with last modified time
        bool needsRebuild = true;            // Flag indicating if module needs rebuilding
        AngelScriptEngine* engine = nullptr; // Reference to parent engine
        std::vector<std::string> defineWords; // Defined words for conditional compilation
        
        /**
         * Constructor
         */
        ScriptModule(const std::string& name, AngelScriptEngine* engine) 
            : name(name), engine(engine), module(nullptr), needsRebuild(true) {
            // Create the script builder
            builder = std::make_shared<CScriptBuilder>();
        }

        /**
         * Check if module is valid
         */
        bool isValid() const { 
            return module != nullptr && !needsRebuild; 
        }

        /**
         * Add a script file to this module
         * @return Result object indicating success or failure
         */
        Result addScriptFile(const std::string& scriptFile) {
            std::string fullPath = engine->findScriptFile(scriptFile);
            if (fullPath.empty()) {
                std::string errorMsg = "Script file not found: " + scriptFile;
                engine->log(LogLevel::Error, errorMsg);
                return Result::Error(errorMsg);
            }

            try {
                // Track the file for hot reloading
                scriptFiles[fullPath] = std::filesystem::last_write_time(fullPath);
                needsRebuild = true;
                
                engine->log(LogLevel::Debug, "Added script file to module " + name + ": " + fullPath);
                return Result::Ok();
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::string errorMsg = "Filesystem error when adding script " + 
                    scriptFile + ": " + e.what();
                engine->log(LogLevel::Error, errorMsg);
                return Result::Error(errorMsg);
            }
        }

        /**
         * Add script code directly to this module
         * @return Result object indicating success or failure
         */
        Result addScriptCode(const std::string& sectionName, const std::string& code) {
            // We'll store the code for later when we build
            needsRebuild = true;
            
            // We don't actually add the code here, because we recreate the builder
            // when we build the module. We'll add a special entry in scriptFiles with
            // a "memory:" prefix to indicate this is a memory section, not a file.
            std::string memoryKey = "memory:" + sectionName;
            
            // Store the code in a temporary file that will be loaded during build
            std::string tempFile = engine->getTempDirectory() + "/" + sectionName + ".as";
            try {
                std::ofstream file(tempFile);
                if (!file.is_open()) {
                    std::string errorMsg = "Failed to create temporary file for script code: " + tempFile;
                    engine->log(LogLevel::Error, errorMsg);
                    return Result::Error(errorMsg);
                }
                
                file << code;
                file.close();
                
                // Track this file for hot reloading
                scriptFiles[tempFile] = std::filesystem::last_write_time(tempFile);
                
                engine->log(LogLevel::Debug, "Added script code to module " + name + ": " + sectionName);
                return Result::Ok();
            }
            catch (const std::exception& e) {
                std::string errorMsg = "Failed to add script code: " + std::string(e.what());
                engine->log(LogLevel::Error, errorMsg);
                return Result::Error(errorMsg);
            }
        }
        
        /**
         * Get metadata declared for a type
         */
        std::vector<std::string> getMetadataForType(int typeId) {
            if (!builder) {
                engine->log(LogLevel::Warning, "Module not built yet, can't get metadata");
                return {};
            }
            return builder->GetMetadataForType(typeId);
        }
        
        /**
         * Get metadata declared for a function
         */
        std::vector<std::string> getMetadataForFunction(asIScriptFunction* func) {
            if (!builder) {
                engine->log(LogLevel::Warning, "Module not built yet, can't get metadata");
                return {};
            }
            return builder->GetMetadataForFunc(func);
        }
        
        /**
         * Get metadata declared for a global variable
         */
        std::vector<std::string> getMetadataForVariable(int varIdx) {
            if (!builder) {
                engine->log(LogLevel::Warning, "Module not built yet, can't get metadata");
                return {};
            }
            return builder->GetMetadataForVar(varIdx);
        }
        
        /**
         * Get metadata declared for a class method
         */
        std::vector<std::string> getMetadataForTypeMethod(int typeId, asIScriptFunction* method) {
            if (!builder) {
                engine->log(LogLevel::Warning, "Module not built yet, can't get metadata");
                return {};
            }
            return builder->GetMetadataForTypeMethod(typeId, method);
        }
        
        /**
         * Get metadata declared for a class property
         */
        std::vector<std::string> getMetadataForTypeProperty(int typeId, int varIdx) {
            if (!builder) {
                engine->log(LogLevel::Warning, "Module not built yet, can't get metadata");
                return {};
            }
            return builder->GetMetadataForTypeProperty(typeId, varIdx);
        }
        
        /**
         * Define a word for conditional compilation
         */
        void defineWord(const std::string& word) {
            defineWords.push_back(word);
            needsRebuild = true;
        }
        
        /**
         * Build/compile this module
         * @return Result object indicating success or failure
         */
        Result build() {
            try {
                // Create a new builder
                builder = std::make_shared<CScriptBuilder>();
                
                // Add defined words for conditional compilation
                for (const auto& word : defineWords) {
                    builder->DefineWord(word.c_str());
                }
                
                // Start a new module
                int r = builder->StartNewModule(engine->getEngine(), name.c_str());
                if (r < 0) {
                    std::string errorMsg = "Failed to start new module: " + name + 
                        " (error code: " + std::to_string(r) + ")";
                    engine->log(LogLevel::Error, errorMsg);
                    return Result::Error(errorMsg);
                }
                
                // Add all script files to the builder
                for (const auto& [filePath, _] : scriptFiles) {
                    r = builder->AddSectionFromFile(filePath.c_str());
                    if (r < 0) {
                        std::string errorMsg = "Failed to add script section from file: " + filePath + 
                            " (error code: " + std::to_string(r) + ")";
                        engine->log(LogLevel::Error, errorMsg);
                        return Result::Error(errorMsg);
                    }
                }
                
                // Build the module
                r = builder->BuildModule();
                if (r < 0) {
                    std::string errorMsg = "Failed to build module: " + name + 
                        " (error code: " + std::to_string(r) + ")";
                    engine->log(LogLevel::Error, errorMsg);
                    return Result::Error(errorMsg);
                }
                
                // Store the module
                module = builder->GetModule();
                
                // After successful build, track all included files for hot reloading
                trackIncludedFiles();
                
                needsRebuild = false;
                engine->log(LogLevel::Info, "Successfully built module: " + name);
                return Result::Ok();
            }
            catch (const std::exception& e) {
                std::string errorMsg = "Exception during build of module " + 
                    name + ": " + e.what();
                engine->log(LogLevel::Error, errorMsg);
                return Result::Error(errorMsg);
            }
        }
        
        /**
         * Track all files included during the build for hot reloading
         */
        void trackIncludedFiles() {
            if (!builder) return;
            
            // Get all sections from the builder
            unsigned int sectionCount = builder->GetSectionCount();
            for (unsigned int i = 0; i < sectionCount; i++) {
                std::string sectionName = builder->GetSectionName(i);
                
                // Skip memory sections (they don't have a file on disk)
                if (sectionName.substr(0, 7) == "memory:") continue;
                
                // Check if this is a file that exists
                try {
                    if (std::filesystem::exists(sectionName)) {
                        // Add to tracking if not already tracked
                        if (scriptFiles.find(sectionName) == scriptFiles.end()) {
                            scriptFiles[sectionName] = std::filesystem::last_write_time(sectionName);
                            engine->log(LogLevel::Debug, "Tracking included file for hot reload: " + sectionName);
                        }
                    }
                } catch (const std::exception& e) {
                    engine->log(LogLevel::Warning, "Failed to track included file: " + 
                        sectionName + " - " + e.what());
                }
            }
        }


        /**
         * Check if any script files have been modified and mark for rebuild if needed
         */
        bool checkForModifiedFiles() {
            bool modified = false;
            const int stableTimeMs = 100; // Time a file should be stable before considering it modified

            // Check if any script files have been modified
            for (auto& [filePath, lastModified] : scriptFiles) {
                try {
                    auto currentModified = std::filesystem::last_write_time(filePath);
                    
                    // If the file was modified, wait a bit to ensure it's done being written
                    if (currentModified != lastModified) {
                        // Record the current time
                        auto checkTime = std::chrono::steady_clock::now();
                        
                        // Wait briefly to let the file stabilize
                        std::this_thread::sleep_for(std::chrono::milliseconds(stableTimeMs));
                        
                        // Check if it changed again during our wait
                        auto newModified = std::filesystem::last_write_time(filePath);
                        if (newModified != currentModified) {
                            // File is still being written, don't trigger rebuild yet
                            engine->log(LogLevel::Debug, "File still being modified, delaying rebuild: " + filePath);
                            continue;
                        }
                        
                        // Get file size to make sure it's not empty
                        auto fileSize = std::filesystem::file_size(filePath);
                        if (fileSize == 0) {
                            engine->log(LogLevel::Warning, "Modified file is empty, delaying rebuild: " + filePath);
                            continue;
                        }
                        
                        // Passed all checks, record the modification
                        lastModified = newModified;
                        modified = true;
                        engine->log(LogLevel::Info, "Detected change in file: " + filePath);
                    }
                }
                catch (const std::filesystem::filesystem_error& e) {
                    // File might be temporarily unavailable, skip it
                    engine->log(LogLevel::Debug, "Error checking file modification time: " + 
                        std::string(e.what()) + " for " + filePath);
                    continue;
                }
            }

            if (modified) {
                needsRebuild = true;
            }

            return modified;
        }

        /**
         * Get a function from this module
         */
        asIScriptFunction* getFunction(const std::string& functionDecl) {
            if (!module) {
                return nullptr;
            }
            return module->GetFunctionByDecl(functionDecl.c_str());
        }

        /**
         * Save compiled bytecode to file for faster loading next time
         */
        Result saveBytecode(const std::string& filename) {
            if (!module) {
                return Result::Error("No module to save");
            }

            // Make sure the module is built
            if (needsRebuild) {
                auto result = build();
                if (!result) {
                    return result;
                }
            }

            // Create a CBytecodeStream instance
            CBytecodeStream stream;
            
            // Save the bytecode to the stream
            int r = module->SaveByteCode(&stream);
            if (r < 0) {
                return Result::Error("Failed to save bytecode (error code: " + std::to_string(r) + ")");
            }

            // Write the stream to a file
            if (!stream.SaveToFile(filename)) {
                return Result::Error("Failed to write bytecode to file: " + filename);
            }

            return Result::Ok();
        }

        /**
         * Load compiled bytecode from file
         */
        Result loadBytecode(const std::string& filename) {
            // Create the module if it doesn't exist
            if (!module) {
                module = engine->getEngine()->GetModule(name.c_str(), asGM_ALWAYS_CREATE);
                if (!module) {
                    return Result::Error("Failed to create module: " + name);
                }
            }
            else {
                // Discard the module to rebuild it
                module = engine->getEngine()->GetModule(name.c_str(), asGM_ALWAYS_CREATE);
            }

            // Create a CBytecodeStream instance
            CBytecodeStream stream;
            
            // Load the bytecode from file
            if (!stream.LoadFromFile(filename)) {
                return Result::Error("Failed to load bytecode from file: " + filename);
            }

            // Load the bytecode into the module
            int r = module->LoadByteCode(&stream);
            if (r < 0) {
                return Result::Error("Failed to load bytecode (error code: " + std::to_string(r) + ")");
            }

            needsRebuild = false;
            return Result::Ok();
        }

        /**
         * Execute a function in this module
         */
        Result executeFunction(
            const std::string& functionDecl,
            const std::vector<ScriptFunctionCallback>& argCallbacks = {},
            std::chrono::milliseconds timeout = std::chrono::milliseconds(0)
        ) {
            // Make sure the module is built and up to date
            if (needsRebuild) {
                auto result = build();
                if (!result) {
                    return result;
                }
            }

            // Find the function
            asIScriptFunction* func = getFunction(functionDecl);
            if (!func) {
                return Result::Error("Function not found: " + functionDecl + " in module " + name);
            }

            // Get a context and execute
            auto ctx = engine->createContext();
            if (!ctx) {
                return Result::Error("Failed to create script context");
            }

            return ctx->execute(func, argCallbacks, timeout);
        }

        /**
         * Helper class for bytecode stream operations
         */
        class CBytecodeStream : public asIBinaryStream
        {
        public:
            CBytecodeStream() : buffer(nullptr), size(0), position(0) {}
            ~CBytecodeStream() { if (buffer) delete[] buffer; }

            bool SaveToFile(const std::string& filename) {
                if (!buffer || size == 0) return false;
                std::ofstream file(filename, std::ios::binary);
                if (!file.is_open()) return false;
                file.write(reinterpret_cast<const char*>(buffer), size);
                return !file.fail();
            }

            bool LoadFromFile(const std::string& filename) {
                std::ifstream file(filename, std::ios::binary | std::ios::ate);
                if (!file.is_open()) return false;
                
                // Get file size
                size = static_cast<unsigned int>(file.tellg());
                file.seekg(0, std::ios::beg);
                
                // Allocate buffer
                if (buffer) delete[] buffer;
                buffer = new unsigned char[size];
                
                // Read the entire file
                file.read(reinterpret_cast<char*>(buffer), size);
                position = 0;
                
                return !file.fail();
            }

            virtual int Write(const void* ptr, asUINT size) override {
                if (size == 0) return 0;
                
                // Resize buffer if needed
                unsigned int newSize = position + size;
                if (newSize > this->size) {
                    unsigned char* newBuffer = new unsigned char[newSize];
                    if (buffer) {
                        memcpy(newBuffer, buffer, this->size);
                        delete[] buffer;
                    }
                    buffer = newBuffer;
                    this->size = newSize;
                }
                
                // Copy data to buffer
                memcpy(buffer + position, ptr, size);
                position += size;
                
                return 0; // Return 0 on success
            }

            virtual int Read(void* ptr, asUINT size) override {
                if (size == 0) return 0;
                if (position + size > this->size) {
                    // Not enough data in the buffer
                    memset(ptr, 0, size);
                    return -1; // Return -1 to indicate an error
                }
                
                memcpy(ptr, buffer + position, size);
                position += size;
                return 0; // Return 0 on success
            }

        private:
            unsigned char* buffer;
            unsigned int size;
            unsigned int position;
        };
    };

    /**
     * Constructor
     */
    AngelScriptEngine() {
        // Initialize the script engine
        engine = asCreateScriptEngine();
        if (!engine) {
            throw std::runtime_error("Failed to create AngelScript engine");
        }

        // Set the message callback to our internal callback
        engine->SetMessageCallback(asFUNCTION(MessageCallbackFunction), this, asCALL_CDECL);

        // Add the current directory to search paths
        searchPaths.push_back(".");
    }

    /**
     * Destructor - properly clean up all resources
     */
    ~AngelScriptEngine() {
        // Release all pooled contexts
        {
            std::lock_guard<std::mutex> lock(contextPoolMutex);
            while (!contextPool.empty()) {
                auto* ctx = contextPool.front();
                contextPool.pop();
                if (ctx) ctx->Release();
            }
        }

        // Release the engine
        if (engine) {
            engine->ShutDownAndRelease();
            engine = nullptr;
        }
    }

    // Delete copy and move operations to prevent incorrect handling
    AngelScriptEngine(const AngelScriptEngine&) = delete;
    AngelScriptEngine& operator=(const AngelScriptEngine&) = delete;
    AngelScriptEngine(AngelScriptEngine&&) = delete;
    AngelScriptEngine& operator=(AngelScriptEngine&&) = delete;

    /**
     * Set the message callback
     */
    void setMessageCallback(ScriptMessageCallback callback, void* userData = nullptr) {
        messageCallback = callback;
        messageCallbackUserData = userData;
    }

    /**
     * Set a simple message callback function
     */
    void setMessageCallback(void (*callback)(const asSMessageInfo*)) {
        if (!engine) return;
        engine->SetMessageCallback(asFUNCTION(callback), nullptr, asCALL_CDECL);
    }

    /**
     * Set timeout for script execution
     */
    void setDefaultScriptTimeout(std::chrono::milliseconds timeout) {
        defaultScriptTimeout = timeout;
    }

    /**
     * Add a search path for script files
     */
    void addSearchPath(const std::string& path) {
        // Check if the path already exists in the search paths
        if (std::find(searchPaths.begin(), searchPaths.end(), path) == searchPaths.end()) {
            searchPaths.push_back(path);
        }
    }

    /**
     * Get the underlying AngelScript engine
     */
    asIScriptEngine* getEngine() const {
        return engine;
    }

    /**
     * Get all available modules
     */
    std::vector<std::string> getModuleNames() const {
        std::vector<std::string> names;
        names.reserve(modules.size());
        for (const auto& [name, _] : modules) {
            names.push_back(name);
        }
        return names;
    }

    /**
     * Create a new module or get an existing one
     */
    std::shared_ptr<ScriptModule> getModule(const std::string& name, bool createIfNotExists = true) {
        auto it = modules.find(name);
        if (it != modules.end()) {
            return it->second;
        }
        
        if (!createIfNotExists) {
            return nullptr;
        }

        // Create a new module
        auto module = std::make_shared<ScriptModule>(name, this);
        modules[name] = module;
        return module;
    }

    /**
     * Check if any scripts need hot reloading and rebuild them if necessary
     */
    bool checkHotReload() {
        bool anyRebuilt = false;

        for (auto& [_, module] : modules) {
            // Check if any script files have been modified
            if (module->checkForModifiedFiles() && module->needsRebuild) {
                if (module->build()) {
                    anyRebuilt = true;
                }
            }
        }

        return anyRebuilt;
    }

    /**
     * Begin a new configuration group - useful for grouping registrations
     */
    Result beginConfigGroup(const std::string& groupName) {
        int r = engine->BeginConfigGroup(groupName.c_str());
        if (r < 0) {
            return Result::Error("Failed to begin configuration group: " + groupName +
                " (error code: " + std::to_string(r) + ")");
        }
        return Result::Ok();
    }

    /**
     * End current configuration group
     */
    Result endConfigGroup() {
        int r = engine->EndConfigGroup();
        if (r < 0) {
            return Result::Error("Failed to end configuration group (error code: " + std::to_string(r) + ")");
        }
        return Result::Ok();
    }

    /**
     * Remove a configuration group
     */
    Result removeConfigGroup(const std::string& groupName) {
        int r = engine->RemoveConfigGroup(groupName.c_str());
        if (r < 0) {
            return Result::Error("Failed to remove configuration group: " + groupName +
                " (error code: " + std::to_string(r) + ")");
        }
        return Result::Ok();
    }

    /**
     * Set access mask for restricting access to registered entities
     */
    Result setDefaultAccessMask(asDWORD accessMask) {
        int r = engine->SetDefaultAccessMask(accessMask);
        if (r < 0) {
            return Result::Error("Failed to set default access mask (error code: " + std::to_string(r) + ")");
        }
        return Result::Ok();
    }

    /**
     * Register a global function
     * @return Result object indicating success or failure
     */
    Result registerFunction(const std::string& declaration, asSFuncPtr funcPointer, asDWORD callConv = asCALL_CDECL) {
        int r = engine->RegisterGlobalFunction(declaration.c_str(), funcPointer, callConv);
        if (r < 0) {
            std::string error = "Failed to register global function: " + declaration + 
                " (error code: " + std::to_string(r) + ")";
            log(LogLevel::Error, error);
            return Result::Error(error);
        }
        
        log(LogLevel::Debug, "Registered global function: " + declaration);
        return Result::Ok();
    }

    /**
     * Register a global property
     * @return Result object indicating success or failure
     */
    Result registerProperty(const std::string& declaration, void* pointer) {
        int r = engine->RegisterGlobalProperty(declaration.c_str(), pointer);
        if (r < 0) {
            std::string error = "Failed to register global property: " + declaration + 
                " (error code: " + std::to_string(r) + ")";
            log(LogLevel::Error, error);
            return Result::Error(error);
        }
        
        log(LogLevel::Debug, "Registered global property: " + declaration);
        return Result::Ok();
    }

    /**
     * Execute a function by name
     * 
     * @param moduleName The name of the module containing the function
     * @param functionDecl The function declaration (e.g., "void main()")
     * @param args Optional callbacks to set function arguments
     * @param timeout Optional timeout for execution (uses default if not specified)
     * @return Result object indicating success or failure
     */
    Result execute(const std::string& moduleName, const std::string& functionDecl, 
                  const std::vector<ScriptFunctionCallback>& args = {},
                  std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
        
        // Get the module (create if not exists)
        auto module = getModule(moduleName);
        if (!module) {
            std::string error = "Failed to get module: " + moduleName;
            log(LogLevel::Error, error);
            return Result::Error(error);
        }
        
        // Check if any files need reloading
        checkHotReload();
        
        // Execute the function
        auto result = module->executeFunction(functionDecl, args, timeout);
        
        // Log errors if execution failed
        if (!result) {
            log(LogLevel::Error, "Failed to execute " + functionDecl + " in module " + 
                moduleName + ": " + result.message);
        }
        
        return result;
    }

    /**
     * Register an object type
     */
    Result registerObjectType(const std::string& obj, int byteSize, asDWORD flags) {
        int r = engine->RegisterObjectType(obj.c_str(), byteSize, flags);
        if (r < 0) {
            return Result::Error("Failed to register object type: " + obj + 
                " (error code: " + std::to_string(r) + ")");
        }
        return Result::Ok();
    }

    /**
     * Register an object method
     */
    Result registerObjectMethod(const std::string& obj, const std::string& declaration, 
                              asSFuncPtr funcPointer, asDWORD callConv) {
        int r = engine->RegisterObjectMethod(obj.c_str(), declaration.c_str(), funcPointer, callConv);
        if (r < 0) {
            return Result::Error("Failed to register object method: " + obj + "::" + declaration + 
                " (error code: " + std::to_string(r) + ")");
        }
        return Result::Ok();
    }

    /**
     * Register an object property
     */
    Result registerObjectProperty(const std::string& obj, const std::string& declaration, int byteOffset) {
        int r = engine->RegisterObjectProperty(obj.c_str(), declaration.c_str(), byteOffset);
        if (r < 0) {
            return Result::Error("Failed to register object property: " + obj + "::" + declaration + 
                " (error code: " + std::to_string(r) + ")");
        }
        return Result::Ok();
    }

    /**
     * Register an enum
     */
    Result registerEnum(const std::string& type) {
        int r = engine->RegisterEnum(type.c_str());
        if (r < 0) {
            return Result::Error("Failed to register enum: " + type + 
                " (error code: " + std::to_string(r) + ")");
        }
        return Result::Ok();
    }

    /**
     * Register an enum value
     */
    Result registerEnumValue(const std::string& type, const std::string& name, int value) {
        int r = engine->RegisterEnumValue(type.c_str(), name.c_str(), value);
        if (r < 0) {
            return Result::Error("Failed to register enum value: " + type + "::" + name + 
                " (error code: " + std::to_string(r) + ")");
        }
        return Result::Ok();
    }

    /**
     * Register the standard library
     * @return Result object indicating success or failure
     */
    Result registerStdLib() {
        try {
            RegisterStdString(engine);
            RegisterScriptArray(engine, true);
            RegisterStdStringUtils(engine);
            RegisterScriptDictionary(engine);
            RegisterScriptMath(engine);
            RegisterScriptDateTime(engine);
            RegisterScriptFile(engine);
            RegisterScriptFileSystem(engine);
            RegisterScriptAny(engine);
            RegisterScriptHandle(engine);
            RegisterScriptWeakRef(engine);
            RegisterExceptionRoutines(engine);
            log(LogLevel::Debug, "Successfully registered std::string");
            return Result::Ok();
        } catch (const std::exception& e) {
            std::string error = "Failed to register std::string: " + std::string(e.what());
            log(LogLevel::Error, error);
            return Result::Error(error);
        }
    }

    /**
     * Register common primitive types
     */
    Result registerCommonTypes() {
        // Register primitive types and common type definitions
        std::vector<std::pair<std::string, std::string>> typedefs = {
            {"float32", "float"},
            {"float64", "double"},
            {"int8", "int8_t"},
            {"int16", "int16_t"},
            {"int32", "int32_t"},
            {"int64", "int64_t"},
            {"uint8", "uint8_t"},
            {"uint16", "uint16_t"},
            {"uint32", "uint32_t"},
            {"uint64", "uint64_t"}
        };

        for (const auto& [alias, original] : typedefs) {
            int r = engine->RegisterTypedef(alias.c_str(), original.c_str());
            if (r < 0) {
                return Result::Error("Failed to register typedef: " + alias + " -> " + original + 
                    " (error code: " + std::to_string(r) + ")");
            }
        }
        
        return Result::Ok();
    }

    /**
     * Creates a script context from the pool
     */
    std::shared_ptr<ScriptContext> createContext() {
        asIScriptContext* ctx = getContextFromPool();
        if (!ctx) {
            return nullptr;
        }
        return std::make_shared<ScriptContext>(this, ctx);
    }

    /**
     * Find a script file in search paths
     */
    std::string findScriptFile(const std::string& scriptName) {
        // First check if the script name is a direct path that exists
        if (std::filesystem::exists(scriptName)) {
            return scriptName;
        }

        // Check in search paths
        for (const auto& path : searchPaths) {
            std::string fullPath = path + "/" + scriptName;
            if (std::filesystem::exists(fullPath)) {
                return fullPath;
            }
        }

        return ""; // File not found
    }

    /**
     * Load a script file into a string with retry logic
     */
    std::string loadScriptFile(const std::string& filePath) {
        // Retry parameters
        const int maxRetries = 3;
        const std::chrono::milliseconds retryDelay(50);
        
        std::string content;
        std::exception lastException;
        bool lastExceptionValid = false;
        
        // Try multiple times to handle possible file locks during saving
        for (int attempt = 0; attempt < maxRetries; ++attempt) {
            try {
                // Check file exists and has size
                std::filesystem::path path(filePath);
                if (!std::filesystem::exists(path)) {
                    if (attempt < maxRetries - 1) {
                        log(LogLevel::Debug, "File not found on attempt " + std::to_string(attempt+1) + 
                            ", retrying: " + filePath);
                        std::this_thread::sleep_for(retryDelay);
                        continue;
                    }
                    log(LogLevel::Error, "File not found after " + std::to_string(attempt+1) + 
                        " attempts: " + filePath);
                    return "";
                }
                
                if (std::filesystem::file_size(path) == 0) {
                    if (attempt < maxRetries - 1) {
                        log(LogLevel::Debug, "File is empty on attempt " + std::to_string(attempt+1) + 
                            ", retrying: " + filePath);
                        std::this_thread::sleep_for(retryDelay);
                        continue;
                    }
                    log(LogLevel::Warning, "File is empty: " + filePath);
                    return "";
                }
                
                // Open and read file
                std::ifstream file(filePath, std::ios::binary | std::ios::ate);
                if (!file.is_open()) {
                    if (attempt < maxRetries - 1) {
                        log(LogLevel::Debug, "Failed to open file on attempt " + std::to_string(attempt+1) + 
                            ", retrying: " + filePath);
                        std::this_thread::sleep_for(retryDelay);
                        continue;
                    }
                    log(LogLevel::Error, "Failed to open file after " + std::to_string(attempt+1) + 
                        " attempts: " + filePath);
                    return "";
                }

                std::streamsize size = file.tellg();
                if (size == 0) {
                    file.close();
                    if (attempt < maxRetries - 1) {
                        log(LogLevel::Debug, "File is empty (at read time) on attempt " + 
                            std::to_string(attempt+1) + ", retrying: " + filePath);
                        std::this_thread::sleep_for(retryDelay);
                        continue;
                    }
                    log(LogLevel::Warning, "File is empty (at read time): " + filePath);
                    return "";
                }
                
                file.seekg(0, std::ios::beg);
                
                content.resize(size);
                if (!file.read(&content[0], size)) {
                    file.close();
                    if (attempt < maxRetries - 1) {
                        log(LogLevel::Debug, "Failed to read file on attempt " + std::to_string(attempt+1) + 
                            ", retrying: " + filePath);
                        std::this_thread::sleep_for(retryDelay);
                        continue;
                    }
                    log(LogLevel::Error, "Failed to read file after " + std::to_string(attempt+1) + 
                        " attempts: " + filePath);
                    return "";
                }
                
                // Success!
                file.close();
                if (attempt > 0) {
                    log(LogLevel::Info, "Successfully loaded file on attempt " + 
                        std::to_string(attempt+1) + ": " + filePath);
                }
                return content;
            }
            catch (const std::exception& e) {
                lastException = e;
                lastExceptionValid = true;
                
                if (attempt < maxRetries - 1) {
                    log(LogLevel::Debug, "Exception when loading file on attempt " + 
                        std::to_string(attempt+1) + ": " + e.what() + ", retrying...");
                    std::this_thread::sleep_for(retryDelay);
                }
            }
        }
        
        // If we get here, all attempts failed
        std::string errorMsg = "Failed to load file after " + std::to_string(maxRetries) + " attempts";
        if (lastExceptionValid) {
            errorMsg += ": " + std::string(lastException.what());
        }
        log(LogLevel::Error, errorMsg + ": " + filePath);
        return "";
    }

private:
    asIScriptEngine* engine = nullptr;                       // AngelScript engine
    std::map<std::string, std::shared_ptr<ScriptModule>> modules; // Map of script modules
    ScriptMessageCallback messageCallback;                   // Message callback function
    void* messageCallbackUserData = nullptr;                 // User data for message callback
    std::chrono::milliseconds defaultScriptTimeout{1000};    // Default script execution timeout (1 second)
    std::vector<std::string> searchPaths;                    // Search paths for script files
    
    std::string tempDirectory = ".";                         // Directory for temporary files
    
    // Context pool for efficient reuse of contexts
    std::queue<asIScriptContext*> contextPool;
    std::mutex contextPoolMutex;
    int maxContextPoolSize = 10;  // Maximum number of contexts to keep in the pool
    
public:
    /**
     * Set the directory for temporary files
     */
    void setTempDirectory(const std::string& directory) {
        tempDirectory = directory;
    }
    
    /**
     * Get the directory for temporary files
     */
    std::string getTempDirectory() const {
        return tempDirectory;
    }

    /**
     * Get a context from the pool or create a new one
     */
    asIScriptContext* getContextFromPool() {
        std::lock_guard<std::mutex> lock(contextPoolMutex);
        
        // Check if we have a context in the pool
        if (!contextPool.empty()) {
            asIScriptContext* ctx = contextPool.front();
            contextPool.pop();
            return ctx;
        }
        
        // Create a new context
        return engine->CreateContext();
    }

    /**
     * Return a context to the pool
     */
    void returnContextToPool(asIScriptContext* ctx) {
        if (!ctx) return;
        
        std::lock_guard<std::mutex> lock(contextPoolMutex);
        
        // Reset the context for reuse
        ctx->Unprepare();
        
        // Add to pool if we haven't reached max size
        if (contextPool.size() < maxContextPoolSize) {
            contextPool.push(ctx);
        } else {
            // Otherwise release it
            ctx->Release();
        }
    }

    /**
     * Internal message callback that forwards to user-defined callback
     */
    static void MessageCallbackFunction(const asSMessageInfo* msg, void* param) {
        AngelScriptEngine* engine = static_cast<AngelScriptEngine*>(param);
        if (engine && engine->messageCallback) {
            engine->messageCallback(msg, engine->messageCallbackUserData);
        }
        else {
            // Default behavior if no callback is set
            const char* type = "ERR ";
            if (msg->type == asMSGTYPE_WARNING) type = "WARN";
            else if (msg->type == asMSGTYPE_INFORMATION) type = "INFO";
            
            std::cerr << msg->section << " (" << msg->row << ", " << msg->col << ") : " 
                      << type << " : " << msg->message << std::endl;
        }
    }
};