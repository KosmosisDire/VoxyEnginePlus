# PrismEngine/CMakeLists.txt - Rewritten
cmake_minimum_required(VERSION 3.21)

# --- Source Files ---
# Glob sources for the main application
file(GLOB_RECURSE PRISM_ENGINE_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp" # Headers often not needed for executable sources
    "${CMAKE_CURRENT_SOURCE_DIR}/src/user-interface/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/user-interface/**/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/voxels/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/voxels/**/*.hpp"
)

# --- Executable Definition ---
# Use PROJECT_NAME defined in the root CMakeLists.txt
add_executable(${PROJECT_NAME} ${PRISM_ENGINE_SRC})

# --- Include Directories ---
# Include directories needed specifically by PrismEngine sources
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    # Add other private include paths if necessary
)

# --- Linking ---
# Link against the Mycelium static library
# Public/Private linking depends on usage requirements propagation.
# If PrismEngine doesn't expose Mycelium headers/features in its own interface, PRIVATE is fine.
# If it does, PUBLIC might be needed, but Mycelium already links its deps PUBLICly. Let's use PRIVATE.
target_link_libraries(${PROJECT_NAME} PRIVATE Mycelium)

# --- Resource Handling ---
# Define source resource directories (can be relative to CMAKE_SOURCE_DIR)
set(RESOURCE_DIRS
    "${CMAKE_SOURCE_DIR}/Mycelium/resources"
    "${CMAKE_SOURCE_DIR}/PrismEngine/resources"
)

# Define the destination directory relative to the executable
set(RESOURCE_DEST_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources")

# Post-build command to copy/link resources
# Using cmake -E commands for cross-platform compatibility
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${RESOURCE_DEST_DIR}"
    COMMENT "Ensuring resource destination directory exists: ${RESOURCE_DEST_DIR}"
    VERBATIM
)

foreach(RES_SRC_DIR ${RESOURCE_DIRS})
    # Always copy contents directly into the target resource directory to merge them.
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${RES_SRC_DIR}" "${RESOURCE_DEST_DIR}" # Copy contents directly
        COMMENT "Copying resources: ${RES_SRC_DIR} -> ${RESOURCE_DEST_DIR}"
        VERBATIM
    )
endforeach()

# --- DLL Handling (Windows) ---
# Copy necessary runtime DLLs next to the executable (CMake 3.21+)
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS # Important!
        COMMENT "Copying runtime DLLs to $<TARGET_FILE_DIR:${PROJECT_NAME}>"
        VERBATIM
    )
endif()

message(STATUS "PrismEngine executable configuration complete.")
