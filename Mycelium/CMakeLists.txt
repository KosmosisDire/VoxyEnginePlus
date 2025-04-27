# Mycelium/CMakeLists.txt
cmake_minimum_required(VERSION 3.29)

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

# --- Find vcpkg Dependencies ---
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED COMPONENTS glfw-binding docking-experimental)

# --- Find Slang ---
find_library(MYCELIUM_SLANG_LIB_PATH slang)
if(NOT MYCELIUM_SLANG_LIB_PATH)
    message(FATAL_ERROR "Mycelium: Could not find the Slang shared library (libslang.so / slang.dll). ")
endif()

# --- Source File Collection ---
file(GLOB_RECURSE MYCELIUM_REGULAR_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/**/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/**/*.hpp"
)


# --- Include Directories --- (Unchanged)
target_include_directories(Mycelium PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/resources>
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
    @ONLY
)