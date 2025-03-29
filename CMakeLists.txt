# Root CMakeLists.txt
cmake_minimum_required(VERSION 3.19)

# Set RPATH handling early before project is defined
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib")

# Include dependencies
include("${CMAKE_CURRENT_LIST_DIR}/cmake/deps.cmake")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# Define the project
project(VoxyEngine VERSION 1.0)

# Add engine library first
add_subdirectory(Mycelium)

# Add main application
add_subdirectory(PrismEngine)
