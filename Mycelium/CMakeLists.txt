include("${CMAKE_CURRENT_LIST_DIR}/deps.cmake")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "lib/vcpkg/installed/x64-windows/share/glfw3")

# Collect Engine Sources
file(GLOB_RECURSE ENGINE_ONLY_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp"
)

# Create static library
add_library(Mycelium STATIC
    ${ENGINE_ONLY_SRC}
)


# Configure project_paths.h
# Assume project_paths.h.in is in Mycelium/include/
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/project_paths.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/project_paths.h"
)

# Define Include Directories for Mycelium
target_include_directories(Mycelium
    PUBLIC # Things users of Mycelium need
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${PROJECT_BINARY_DIR}/include
)

# Find and link dependencies
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# Link dependencies
target_link_libraries(Mycelium
    PUBLIC
        glfw
        daxa::daxa
        glm::glm
        fmt::fmt
        imgui::imgui
)
