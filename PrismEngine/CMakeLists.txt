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
    "${CMAKE_CURRENT_SOURCE_DIR}/resources"
)

# --- Linking ---
# Link against the Mycelium static library
# Public/Private linking depends on usage requirements propagation.
# If PrismEngine doesn't expose Mycelium headers/features in its own interface, PRIVATE is fine.
# If it does, PUBLIC might be needed, but Mycelium already links its deps PUBLICly. Let's use PRIVATE.
target_link_libraries(${PROJECT_NAME} PUBLIC Mycelium)

# Add SSE2 support for Clang (needed for intrinsics like in mmintrin.h)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -msse2)
endif()

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


# Find Slang library path again (needed for the copy command)
find_library(PRISM_ENGINE_SLANG_LIB_PATH slang)
if(NOT PRISM_ENGINE_SLANG_LIB_PATH)
    message(ERROR "PrismEngine: Could not find Slang shared library for copying. Runtime issues may occur if not handled otherwise.")
else()
    message(STATUS "PrismEngine: Found Slang library to copy: ${PRISM_ENGINE_SLANG_LIB_PATH}")
    # Copy Slang shared library (.so/.dylib) next to executable for RPATH
    if(UNIX) # Handles Linux (.so) and macOS (.dylib)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${PRISM_ENGINE_SLANG_LIB_PATH}"
                    $<TARGET_FILE_DIR:${PROJECT_NAME}> # Copy to executable directory
            COMMENT "Copying Slang shared library (${PRISM_ENGINE_SLANG_LIB_PATH}) for runtime"
            VERBATIM
        )
    endif()
endif()

# Copy necessary runtime DLLs next to the executable (Windows)
# This might include slang.dll if found and linked as a runtime dependency.
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime DLLs to $<TARGET_FILE_DIR:${PROJECT_NAME}>"
        VERBATIM
    )
endif()

message(STATUS "PrismEngine executable configuration complete.")
