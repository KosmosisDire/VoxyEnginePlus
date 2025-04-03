include(FetchContent)
FetchContent_Declare(
    antlr
    GIT_REPOSITORY https://github.com/antlr/antlr4.git
    GIT_TAG        4.13.2
)
FetchContent_MakeAvailable(antlr)
message(STATUS "Mycelium: antlr_SOURCE_DIR = ${antlr_SOURCE_DIR}")

FetchContent_Declare(
    angelscript
    GIT_REPOSITORY https://github.com/codecat/angelscript-mirror.git
    GIT_BRANCH     master
)
FetchContent_MakeAvailable(angelscript)
message(STATUS "Mycelium: angelscript_SOURCE_DIR = ${angelscript_SOURCE_DIR}")


# FetchContent_Declare(
#     daxa
#     GIT_REPOSITORY https://github.com/Ipotrick/Daxa.git
#     GIT_TAG        3.0.4
# )
# FetchContent_MakeAvailable(daxa)
# message(STATUS "Mycelium: daxa_SOURCE_DIR = ${daxa_SOURCE_DIR}")

if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../lib/Daxa/CMakeLists.txt")
    find_package(Git REQUIRED)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../lib")
    execute_process(COMMAND ${GIT_EXECUTABLE} clone https://github.com/Ipotrick/Daxa
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../lib"
        COMMAND_ERROR_IS_FATAL ANY)
    execute_process(COMMAND ${GIT_EXECUTABLE} checkout d91f215842ca0d49cc6882a234172fdeb5b383b7
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../lib/Daxa"
        COMMAND_ERROR_IS_FATAL ANY)
endif()

# If the user has set a toolchain file, we'll want to chainload it via vcpkg
if(NOT (CMAKE_TOOLCHAIN_FILE MATCHES "/scripts/buildsystems/vcpkg.cmake") AND DEFINED CMAKE_TOOLCHAIN_FILE)
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}" CACHE UNINITIALIZED "")
endif()

# Check if vcpkg is installed globally. Otherwise, clone vcpkg
if(EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    file(TO_CMAKE_PATH $ENV{VCPKG_ROOT} VCPKG_ROOT)
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
else()
    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../lib/vcpkg/scripts/buildsystems/vcpkg.cmake")
        find_package(Git REQUIRED)
        execute_process(COMMAND ${GIT_EXECUTABLE} clone https://github.com/Microsoft/vcpkg
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../lib"
            COMMAND_ERROR_IS_FATAL ANY)
    endif()
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/../lib/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()










