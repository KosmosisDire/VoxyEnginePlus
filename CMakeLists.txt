# Root CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Set RPATH handling early before project is defined
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# Define the project
project(PrismEngine VERSION 1.0)

# --- Fetch Dependencies ---
include(FetchContent)
FetchContent_Declare(
    antlr
    GIT_REPOSITORY https://github.com/antlr/antlr4.git
    GIT_TAG        4.13.2 # Use the same tag as before
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE) # Force static libs for dependencies
set(ANTLR_BUILD_CPP_RUNTIME ON CACHE BOOL "Build ANTLR C++ Runtime" FORCE) # Ensure runtime is built
set(WITH_STATIC_CRT OFF CACHE BOOL "Build ANTLR with dynamic CRT" FORCE) # Match project CRT
FetchContent_MakeAvailable(antlr)
message(STATUS "Root: antlr_SOURCE_DIR = ${antlr_SOURCE_DIR}") # For debugging

# Add engine library first
add_subdirectory(Mycelium)

# Add main application
add_subdirectory(PrismEngine)
