# Mycelium/deps.cmake
# Dependencies fetched specifically for Mycelium

include(FetchContent) # Ensure FetchContent is available

# --- ANTLR ---
FetchContent_Declare(
    antlr
    GIT_REPOSITORY https://github.com/antlr/antlr4.git
    GIT_TAG        4.13.2 # Use the same tag as before
)
# Set ANTLR build options *before* MakeAvailable
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE) # Force static libs for dependencies
set(ANTLR_BUILD_CPP_RUNTIME ON CACHE BOOL "Build ANTLR C++ Runtime" FORCE) # Ensure runtime is built
set(WITH_STATIC_CRT OFF CACHE BOOL "Build ANTLR with dynamic CRT" FORCE) # Match project CRT
FetchContent_MakeAvailable(antlr)
message(STATUS "Mycelium: antlr_SOURCE_DIR = ${antlr_SOURCE_DIR}")

# --- AngelScript ---
FetchContent_Declare(
    angelscript
    GIT_REPOSITORY https://github.com/codecat/angelscript-mirror.git
    GIT_BRANCH     master
)
FetchContent_MakeAvailable(angelscript)
message(STATUS "Mycelium: angelscript_SOURCE_DIR = ${angelscript_SOURCE_DIR}")

# --- Daxa ---
FetchContent_Declare(
    daxa
    GIT_REPOSITORY https://github.com/Ipotrick/Daxa.git
    GIT_TAG        d91f215842ca0d49cc6882a234172fdeb5b383b7 # Specific commit requested
)
# Set Daxa build options *before* MakeAvailable
set(DAXA_ENABLE_UTILS_IMGUI ON CACHE BOOL "Enable Daxa ImGui utils" FORCE)
set(DAXA_ENABLE_UTILS_MEM ON CACHE BOOL "Enable Daxa memory utils" FORCE)
set(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_GLSLANG ON CACHE BOOL "Enable Daxa pipeline manager with GLSLang" FORCE)
set(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_SLANG ON CACHE BOOL "Enable Daxa pipeline manager with Slang" FORCE)
set(DAXA_ENABLE_UTILS_TASK_GRAPH ON CACHE BOOL "Enable Daxa task graph utils" FORCE)
# Add other Daxa options if needed, e.g.:
# set(DAXA_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(daxa)
message(STATUS "Mycelium: daxa_SOURCE_DIR = ${daxa_SOURCE_DIR}")

# vcpkg setup is now handled in the root CMakeLists.txt
# Manual Daxa clone is replaced by FetchContent above
