# Mycelium/CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

include(FetchContent)

# Daxa
FetchContent_Declare(
    daxa
    GIT_REPOSITORY https://github.com/Ipotrick/Daxa.git
    GIT_TAG        d91f215842ca0d49cc6882a234172fdeb5b383b7
)
set(DAXA_ENABLE_UTILS_IMGUI ON)
set(DAXA_ENABLE_UTILS_MEM ON)
set(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_SLANG ON)
set(DAXA_ENABLE_UTILS_TASK_GRAPH ON)
FetchContent_MakeAvailable(daxa)
message(STATUS "Mycelium: Using Daxa source from ${daxa_SOURCE_DIR}")

# --- Find vcpkg Dependencies ---
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED COMPONENTS glfw-binding docking-experimental)

find_library(MYCELIUM_SLANG_LIB_PATH slang)
if(NOT MYCELIUM_SLANG_LIB_PATH)
    message(FATAL_ERROR "Mycelium: Could not find the Slang shared library (libslang.so / slang.dll). "
                        "Ensure Slang is installed correctly (e.g., via vcpkg or system) "
                        "and the CMake environment (toolchain) is set up.")
endif()
message(STATUS "Mycelium: Found Slang library at ${MYCELIUM_SLANG_LIB_PATH}")

# --- Source Files ---
# Glob sources - consider listing explicitly for better control if needed
file(GLOB_RECURSE MYCELIUM_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp" # Headers often not needed, but included in original
    "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/**/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/**/*.hpp"
    # Scripting directory removed from sources
)

# --- Library Definition ---
add_library(Mycelium STATIC
    ${MYCELIUM_SRC}
)

# --- Include Directories ---
target_include_directories(Mycelium PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/resources> 
)
target_include_directories(Mycelium PRIVATE
)

# --- Compile Definitions ---
target_compile_definitions(Mycelium PUBLIC
)
target_compile_definitions(Mycelium PRIVATE
)


# --- Linking ---
target_link_libraries(Mycelium PUBLIC
    glfw
    glm::glm
    fmt::fmt
    imgui::imgui
    daxa::daxa
    ${MYCELIUM_SLANG_LIB_PATH}
)

# --- Configure project_paths.h ---
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/project_paths.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/project_paths.h"
    @ONLY # Process only @VAR@ variables if used
)

message(STATUS "Mycelium library configuration complete.")
