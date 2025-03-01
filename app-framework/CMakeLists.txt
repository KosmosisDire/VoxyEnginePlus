# app-framework/CMakeLists.txt
file(GLOB_RECURSE ENGINE_SRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Create static library
add_library(voxy_engine STATIC ${ENGINE_SRC})

# Configure engine_config.hpp with resource path
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/engine_config.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/engine_config.hpp"
    @ONLY
)

# Add include directories
target_include_directories(voxy_engine 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Find and link dependencies
find_package(glfw3 CONFIG REQUIRED)
find_package(daxa CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# Link dependencies
target_link_libraries(voxy_engine 
    PUBLIC 
        glfw
        daxa::daxa
        glm::glm
        fmt::fmt
        imgui::imgui
)

# Create resources directories
add_custom_command(TARGET voxy_engine PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders"
    COMMENT "Creating engine resource directories"
)