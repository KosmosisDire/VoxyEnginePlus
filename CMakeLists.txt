# Root CMakeLists.txt - Rewritten
cmake_minimum_required(VERSION 3.21)

# --- Project Definition ---
project(PrismEngine VERSION 1.0 LANGUAGES CXX C)

# --- Ensure compile_commands.json is generated ---
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- C++ Standard ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF) # Prefer explicit standard features

# --- Compiler Specific Flags ---
# Enable UTF-8 source/execution character sets consistently
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:Clang>>:-finput-charset=UTF-8>")
add_compile_options("$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-finput-charset=UTF-8>")
# Consider adding warning flags here (e.g., /W4 for MSVC, -Wall -Wextra for Clang/GCC)

# --- vcpkg Toolchain Setup ---
# Set the toolchain file ONLY if it's not already defined (e.g., by preset or command line)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE OR CMAKE_TOOLCHAIN_FILE STREQUAL "")
    set(LOCAL_VCPKG_TOOLCHAIN "${CMAKE_SOURCE_DIR}/lib/vcpkg/scripts/buildsystems/vcpkg.cmake")
    if(EXISTS "${LOCAL_VCPKG_TOOLCHAIN}")
        set(CMAKE_TOOLCHAIN_FILE "${LOCAL_VCPKG_TOOLCHAIN}" CACHE STRING "Vcpkg toolchain file")
        message(STATUS "Using local vcpkg toolchain (set by CMakeLists): ${CMAKE_TOOLCHAIN_FILE}")
    elseif(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}" VCPKG_ROOT_PATH)
        set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT_PATH}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
        message(STATUS "Using global vcpkg toolchain (set by CMakeLists): ${CMAKE_TOOLCHAIN_FILE}")
    else()
        message(WARNING "vcpkg toolchain not found locally (./lib/vcpkg) or globally ($ENV{VCPKG_ROOT}). Set CMAKE_TOOLCHAIN_FILE manually if needed.")
    endif()
else()
    message(STATUS "Using CMAKE_TOOLCHAIN_FILE provided externally: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# If a toolchain file is being used (either set here or externally), check for chainloading
if(DEFINED CMAKE_TOOLCHAIN_FILE AND NOT CMAKE_TOOLCHAIN_FILE STREQUAL "")
    # If the initially provided toolchain file is different from the final one (vcpkg), chainload the original.
    if(DEFINED CMAKE_INITIAL_TOOLCHAIN_FILE AND NOT CMAKE_INITIAL_TOOLCHAIN_FILE STREQUAL CMAKE_TOOLCHAIN_FILE)
        set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_INITIAL_TOOLCHAIN_FILE}" CACHE STRING "Chainload toolchain for vcpkg" FORCE)
        message(STATUS "vcpkg chainloading toolchain: ${VCPKG_CHAINLOAD_TOOLCHAIN_FILE}")
    endif()
endif()


# --- RPATH Handling (Linux/macOS) ---
# Ensure RPATH allows finding libraries relative to the executable
if(UNIX)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) # Embed RPATH in build tree
    # Use $ORIGIN for libraries next to executable or in ../lib
    set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib")
    set(CMAKE_BUILD_RPATH "$ORIGIN:$ORIGIN/../lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) # Add linker paths to RPATH
endif()

# --- FetchContent Setup ---
# Make FetchContent available globally for subprojects
include(FetchContent)

# --- Subdirectories ---
# Add engine library first
add_subdirectory(Mycelium)

# Add main application
add_subdirectory(PrismEngine)

message(STATUS "Root CMake configuration complete.")
