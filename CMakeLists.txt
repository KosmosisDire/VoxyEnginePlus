# Root CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# --- vcpkg Toolchain Setup ---
# If the user has set a toolchain file, we'll want to chainload it via vcpkg
if(NOT (CMAKE_TOOLCHAIN_FILE MATCHES "/scripts/buildsystems/vcpkg.cmake") AND DEFINED CMAKE_TOOLCHAIN_FILE)
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}" CACHE UNINITIALIZED "")
endif()

# Check if vcpkg is installed globally. Otherwise, clone vcpkg
if(EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    file(TO_CMAKE_PATH $ENV{VCPKG_ROOT} VCPKG_ROOT)
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
else()
    set(VCPKG_DIR "${CMAKE_SOURCE_DIR}/lib/vcpkg") # Define vcpkg directory relative to root
    if(NOT EXISTS "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake")
        message(STATUS "vcpkg not found globally or locally. Cloning vcpkg into ${VCPKG_DIR}...")
        find_package(Git REQUIRED)
        # Ensure the parent directory exists before cloning
        get_filename_component(VCPKG_PARENT_DIR "${VCPKG_DIR}" DIRECTORY)
        if(NOT EXISTS "${VCPKG_PARENT_DIR}")
            file(MAKE_DIRECTORY "${VCPKG_PARENT_DIR}")
        endif()
        execute_process(COMMAND ${GIT_EXECUTABLE} clone https://github.com/Microsoft/vcpkg.git "${VCPKG_DIR}"
            COMMAND_ERROR_IS_FATAL ANY)
        # Optional: Bootstrap vcpkg if needed (might depend on workflow)
        # execute_process(COMMAND "${VCPKG_DIR}/bootstrap-vcpkg.bat" -disableMetrics WORKING_DIRECTORY "${VCPKG_DIR}") # Windows example
    endif()
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()
message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
# --- End vcpkg Toolchain Setup ---


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

# Include FetchContent module globally if needed by subprojects
include(FetchContent)

# Add engine library first
add_subdirectory(Mycelium)

# Add main application
add_subdirectory(PrismEngine)
