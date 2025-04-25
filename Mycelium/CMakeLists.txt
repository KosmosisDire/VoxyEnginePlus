# Mycelium/CMakeLists.txt - Rewritten
cmake_minimum_required(VERSION 3.21)

# --- FetchContent Dependencies ---
include(FetchContent)

# ANTLR
FetchContent_Declare(
    antlr
    GIT_REPOSITORY https://github.com/antlr/antlr4.git
    GIT_TAG        4.13.1 # Using 4.13.1, update if needed
)
set(ANTLR_BUILD_CPP_RUNTIME ON CACHE BOOL "" FORCE)
set(WITH_STATIC_CRT OFF CACHE BOOL "" FORCE) # Match project CRT (usually dynamic)
set(ANTLR_JAR_LOCATION "" CACHE FILEPATH "" FORCE) # Don't need the JAR if only building runtime
FetchContent_MakeAvailable(antlr)
message(STATUS "Mycelium: Using ANTLR source from ${antlr_SOURCE_DIR}")

# AngelScript
FetchContent_Declare(
    angelscript
    GIT_REPOSITORY https://github.com/codecat/angelscript-mirror.git # Using mirror
    GIT_TAG        master # Or specific tag/commit if preferred
)
FetchContent_MakeAvailable(angelscript)
message(STATUS "Mycelium: Using AngelScript source from ${angelscript_SOURCE_DIR}")

# Daxa
FetchContent_Declare(
    daxa
    GIT_REPOSITORY https://github.com/Ipotrick/Daxa.git
    GIT_TAG        d91f215842ca0d49cc6882a234172fdeb5b383b7 # Specific commit from original
)
# Set Daxa options BEFORE MakeAvailable
set(DAXA_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(DAXA_ENABLE_UTILS_IMGUI ON CACHE BOOL "" FORCE)
set(DAXA_ENABLE_UTILS_MEM ON CACHE BOOL "" FORCE)
set(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_GLSLANG ON CACHE BOOL "" FORCE)
set(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_SLANG ON CACHE BOOL "" FORCE)
set(DAXA_ENABLE_UTILS_TASK_GRAPH ON CACHE BOOL "" FORCE)
set(DAXA_VALIDATION OFF CACHE BOOL "" FORCE) # Disable validation layers unless needed for debug
set(DAXA_ENABLE_UTILS OFF CACHE BOOL "" FORCE) # Turn off utils master switch if only specific ones are needed? Check Daxa docs. Assuming ON is fine.
FetchContent_MakeAvailable(daxa)
message(STATUS "Mycelium: Using Daxa source from ${daxa_SOURCE_DIR}")

# --- Find vcpkg Dependencies ---
# vcpkg should handle finding these via the toolchain file
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED COMPONENTS glfw-binding docking-experimental) # Specify components

# --- Source Files ---
# Glob sources - consider listing explicitly for better control if needed
file(GLOB_RECURSE MYCELIUM_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp" # Headers often not needed, but included in original
    "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/**/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/**/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/**/*.hpp"
)

# ANTLR Generated Sources (Assuming they land here - adjust path if needed)
# This part might require running the ANTLR generator first.
# For now, let's assume they are pre-generated or handled by another process.
# If ANTLR generation needs to be part of CMake, add custom commands.
set(ANTLR_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/antlr") # Example output dir
file(GLOB_RECURSE ANTLR_GENERATED_SRC "${ANTLR_GENERATED_DIR}/*.cpp")
# Add include directory for generated headers
include_directories(${ANTLR_GENERATED_DIR})

# --- Library Definition ---
add_library(Mycelium STATIC
    ${MYCELIUM_SRC}
    # ANTLR runtime sources will be linked via target_link_libraries
    # AngelScript sources will be linked via target_link_libraries
    # Add ANTLR generated sources if globbed above
    ${ANTLR_GENERATED_SRC}
)

# --- AngelScript Assembly Handling ---
set(AS_USE_ASSEMBLY FALSE)
set(AS_ASM_FILE "")
set(AS_ASM_OBJ "")

if(CMAKE_SIZEOF_VOID_P EQUAL 8) # Only for 64-bit
    if(MSVC)
        set(AS_ASM_FILE_CANDIDATE "${angelscript_SOURCE_DIR}/sdk/angelscript/source/as_callfunc_x64_msvc_asm.asm")
        if(EXISTS "${AS_ASM_FILE_CANDIDATE}")
            # Find ml64.exe (simplified find logic - assumes it's in PATH via Dev Prompt or VS setup)
            find_program(ML64_EXECUTABLE ml64)
            if(ML64_EXECUTABLE)
                message(STATUS "Found ml64 for AngelScript assembly: ${ML64_EXECUTABLE}")
                set(AS_ASM_FILE "${AS_ASM_FILE_CANDIDATE}")
                set(AS_ASM_OBJ "${CMAKE_CURRENT_BINARY_DIR}/as_callfunc_x64_msvc_asm.obj")
                add_custom_command(
                    OUTPUT ${AS_ASM_OBJ}
                    COMMAND ${ML64_EXECUTABLE} /c /nologo /Fo${AS_ASM_OBJ} /W3 /Zi /Ta ${AS_ASM_FILE}
                    DEPENDS ${AS_ASM_FILE}
                    COMMENT "Assembling AngelScript MSVC x64: ${AS_ASM_FILE}"
                    VERBATIM)
                set(AS_USE_ASSEMBLY TRUE)
            else()
                message(WARNING "ml64.exe not found in PATH. AngelScript assembly optimizations disabled for MSVC.")
            endif()
        endif()
    elseif(CMAKE_SYSTEM_NAME MATCHES "Linux" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        set(AS_ASM_FILE_CANDIDATE "${angelscript_SOURCE_DIR}/sdk/angelscript/source/as_callfunc_x64_gcc.S")
        if(EXISTS "${AS_ASM_FILE_CANDIDATE}")
            message(STATUS "Found GCC/Clang compatible AngelScript assembly: ${AS_ASM_FILE_CANDIDATE}")
            set(AS_ASM_FILE "${AS_ASM_FILE_CANDIDATE}")
            set(AS_ASM_OBJ "${CMAKE_CURRENT_BINARY_DIR}/as_callfunc_x64_gcc.o")
            # Use CMAKE_ASM_COMPILER (CMake should find assembler associated with C/CXX compiler)
            add_custom_command(
                OUTPUT ${AS_ASM_OBJ}
                COMMAND ${CMAKE_ASM_COMPILER} ${CMAKE_ASM_FLAGS} -c ${AS_ASM_FILE} -o ${AS_ASM_OBJ}
                DEPENDS ${AS_ASM_FILE}
                COMMENT "Assembling AngelScript GCC/Clang x64: ${AS_ASM_FILE}"
                VERBATIM)
            set(AS_USE_ASSEMBLY TRUE)
        endif()
    # Add elif for macOS Clang if needed (might use the same .S file)
    # elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang") ...
    endif()
endif()

if(AS_USE_ASSEMBLY)
    target_sources(Mycelium PRIVATE ${AS_ASM_OBJ})
    message(STATUS "AngelScript assembly optimizations enabled.")
else()
    target_compile_definitions(Mycelium PRIVATE AS_MAX_PORTABILITY)
    message(STATUS "AngelScript assembly optimizations disabled (using AS_MAX_PORTABILITY).")
endif()

# --- Include Directories ---
target_include_directories(Mycelium PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> # Public headers
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include> # Generated headers (like project_paths.h)
    $<INSTALL_INTERFACE:include> # Install interface
    # Dependencies' public includes
    $<BUILD_INTERFACE:${angelscript_SOURCE_DIR}/sdk/angelscript/include>
    $<BUILD_INTERFACE:${angelscript_SOURCE_DIR}/sdk/> # Add AngelScript SDK root (includes add_on)
    $<BUILD_INTERFACE:${antlr_SOURCE_DIR}/runtime/Cpp/runtime/src> # ANTLR runtime includes
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> # Add Mycelium sources to public interface
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/resources> # Add Mycelium resources to public interface
    # Daxa, glfw, glm, fmt, imgui includes are handled by find_package and target_link_libraries
)
target_include_directories(Mycelium PRIVATE
    # ${CMAKE_CURRENT_SOURCE_DIR}/src # Moved to PUBLIC
    # ${CMAKE_CURRENT_SOURCE_DIR}/resources # Moved to PUBLIC
)

# --- Compile Definitions ---
target_compile_definitions(Mycelium PUBLIC
    ANTLR4CPP_STATIC # For static ANTLR runtime build
    # Add other necessary public definitions
)
target_compile_definitions(Mycelium PRIVATE
    # Add private definitions
)

# --- Compile Options ---
# Force include <chrono> for MSVC ANTLR fix (from original)
if(MSVC)
    target_compile_options(Mycelium PRIVATE /FI"chrono")
endif()

# --- Linking ---
target_link_libraries(Mycelium PUBLIC
    # vcpkg dependencies
    glfw # Try simpler target name based on vcpkg output example
    glm::glm
    fmt::fmt
    imgui::imgui # Use target provided by find_package(imgui)
    # FetchContent dependencies (Daxa provides a target)
    daxa::daxa
    # Link ANTLR runtime library target created by FetchContent
    antlr4_runtime # Assuming this is the target name ANTLR creates
    # Link AngelScript library target created by FetchContent
    angelscript # Assuming this is the target name AngelScript creates
)

# --- Configure project_paths.h ---
# Assumes project_paths.h.in is in Mycelium/include/
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/project_paths.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/project_paths.h"
    @ONLY # Process only @VAR@ variables if used
)

message(STATUS "Mycelium library configuration complete.")
