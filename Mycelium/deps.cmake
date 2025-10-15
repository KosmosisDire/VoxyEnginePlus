# Mycelium/deps.cmake
# Dependencies fetched specifically for Mycelium

include(FetchContent) # Ensure FetchContent is available

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
FetchContent_MakeAvailable(daxa)
message(STATUS "Mycelium: daxa_SOURCE_DIR = ${daxa_SOURCE_DIR}")
