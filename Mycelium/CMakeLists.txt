# Mycelium/CMakeLists.txt - Rewritten
cmake_minimum_required(VERSION 3.21)

# --- FetchContent Dependencies ---
include(FetchContent)

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

# Glob AngelScript sources separately
file(GLOB_RECURSE ANGELSCRIPT_SRC
    "${angelscript_SOURCE_DIR}/sdk/angelscript/source/*.cpp"
    "${angelscript_SOURCE_DIR}/sdk/add_on/**/*.cpp" # Recursively glob all add-on sources
)

# --- Library Definition ---
add_library(Mycelium STATIC
    ${MYCELIUM_SRC}
    ${ANGELSCRIPT_SRC} # Add globbed AngelScript sources
    # ANTLR removed
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
    # Use GCC/Clang assembly on Linux OR Windows if using Clang/GNU compiler
    elseif((CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Windows") AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        set(AS_ASM_FILE_CANDIDATE "${angelscript_SOURCE_DIR}/sdk/angelscript/source/as_callfunc_x64_gcc.S")
        if(EXISTS "${AS_ASM_FILE_CANDIDATE}")
            message(STATUS "Found GCC/Clang compatible AngelScript assembly for ${CMAKE_SYSTEM_NAME}: ${AS_ASM_FILE_CANDIDATE}")
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
    # ANTLR removed
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
    # ANTLR removed
    # Add other necessary public definitions
)
target_compile_definitions(Mycelium PRIVATE
    # Add private definitions
)

# --- Linking ---
target_link_libraries(Mycelium PUBLIC
    # vcpkg dependencies
    glfw # Try simpler target name based on vcpkg output example
    glm::glm
    fmt::fmt
    imgui::imgui # Use target provided by find_package(imgui)
    # FetchContent dependencies (Daxa provides a target)
    daxa::daxa
    # ANTLR removed
    # AngelScript sources are compiled directly into Mycelium now
)

# --- Configure project_paths.h ---
# Assumes project_paths.h.in is in Mycelium/include/
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/project_paths.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/project_paths.h"
    @ONLY # Process only @VAR@ variables if used
)

message(STATUS "Mycelium library configuration complete.")
