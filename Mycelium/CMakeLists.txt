# Mycelium/CMakeLists.txt - Rewritten
cmake_minimum_required(VERSION 3.21)

# --- FetchContent Dependencies ---
include(FetchContent)

# AngelScript Removed

# Daxa
FetchContent_Declare(
    daxa
    GIT_REPOSITORY https://github.com/Ipotrick/Daxa.git
    GIT_TAG        d91f215842ca0d49cc6882a234172fdeb5b383b7 # Specific commit from original
)
# Set Daxa options BEFORE MakeAvailable
set(DAXA_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(DAXA_ENABLE_UTILS_IMGUI ON CACHE BOOL "" FORCE)
set(DAXA_ENABLE_UTILS_MEM ON CACHE BOOL "" FORCE)
set(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_GLSLANG ON CACHE BOOL "" FORCE)
set(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_SLANG ON CACHE BOOL "" FORCE)
set(DAXA_ENABLE_UTILS_TASK_GRAPH ON CACHE BOOL "" FORCE)
set(DAXA_VALIDATION OFF CACHE BOOL "" FORCE) # Disable validation layers unless needed for debug
set(DAXA_ENABLE_UTILS OFF CACHE BOOL "" FORCE) # Turn off utils master switch if only specific ones are needed? Check Daxa docs. Assuming ON is fine.
FetchContent_MakeAvailable(daxa)
message(STATUS "Mycelium: Using Daxa source from ${daxa_SOURCE_DIR}")

# --- Find vcpkg Dependencies ---
# vcpkg should handle finding these via the toolchain file
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED COMPONENTS glfw-binding docking-experimental) # Specify components

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
    # AngelScript removed
    # ANTLR removed
)

# --- Include Directories ---
target_include_directories(Mycelium PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> # Public headers
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include> # Generated headers (like project_paths.h)
    $<INSTALL_INTERFACE:include> # Install interface
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/resources> 
)
target_include_directories(Mycelium PRIVATE
)

# --- Compile Definitions ---
target_compile_definitions(Mycelium PUBLIC
    # ANTLR removed
    # Add other necessary public definitions
)
target_compile_definitions(Mycelium PRIVATE
    # Add private definitions
)

# --- Compile Options ---
# Force include <chrono> for MSVC ANTLR fix (from original)
if(MSVC)
    target_compile_options(Mycelium PRIVATE /FI"chrono")
endif()
# Disable strict aliasing for Clang/GCC as recommended by AngelScript docs
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(Mycelium PRIVATE -fno-strict-aliasing)
endif()

# --- Linking ---
target_link_libraries(Mycelium PUBLIC
    # vcpkg dependencies
    glfw # Try simpler target name based on vcpkg output example
    glm::glm
    fmt::fmt
    imgui::imgui # Use target provided by find_package(imgui)
    # FetchContent dependencies (Daxa provides a target)
    daxa::daxa
    # ANTLR removed
    # AngelScript removed
)

# --- Configure project_paths.h ---
# Assumes project_paths.h.in is in Mycelium/include/
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/project_paths.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/project_paths.h"
    @ONLY # Process only @VAR@ variables if used
)

message(STATUS "Mycelium library configuration complete.")
