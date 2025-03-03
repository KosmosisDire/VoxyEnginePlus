# PrismEngine/CMakeLists.txt
file(GLOB_RECURSE ENGINE_SRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp",
    "${CMAKE_CURRENT_SOURCE_DIR}/include/angelscript/source/*.cpp"
)
# Create static library
add_library(PrismEngine STATIC ${ENGINE_SRC})

if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    # Initialize a variable to hold the ml64 path
    set(ML64 "ML64-NOTFOUND")
    
    # Try to find ml64 in PATH first (works if CMake is run from Developer Command Prompt)
    find_program(ML64_IN_PATH ml64)
    if(ML64_IN_PATH)
        set(ML64 ${ML64_IN_PATH})
    else()
        # Try to use vswhere to locate Visual Studio installation
        find_program(VSWHERE vswhere 
            PATHS "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer" 
            "$ENV{ProgramFiles}/Microsoft Visual Studio/Installer")
            
        if(VSWHERE)
            # Use vswhere to find the latest Visual Studio installation path
            execute_process(
                COMMAND ${VSWHERE} -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
                OUTPUT_VARIABLE VS_INSTALL_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            
            if(VS_INSTALL_DIR)
                # Find the MSVC tools version directory
                file(GLOB MSVC_TOOLS_DIRS "${VS_INSTALL_DIR}/VC/Tools/MSVC/*")
                if(MSVC_TOOLS_DIRS)
                    # Get the latest version (assuming directory naming is consistent)
                    list(SORT MSVC_TOOLS_DIRS)
                    list(GET MSVC_TOOLS_DIRS -1 LATEST_MSVC_TOOLS_DIR)
                    
                    # Check if ml64.exe exists
                    set(ML64_CANDIDATE "${LATEST_MSVC_TOOLS_DIR}/bin/Hostx64/x64/ml64.exe")
                    if(EXISTS "${ML64_CANDIDATE}")
                        set(ML64 "${ML64_CANDIDATE}")
                    endif()
                endif()
            endif()
        endif()
        
        # Fallback method: Try to run vcvarsall.bat to query for ml64
        if(NOT ML64 OR NOT EXISTS "${ML64}")
            # Try to find vcvarsall.bat in common locations
            file(GLOB VS_INSTALLATIONS 
                "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/*/[Community,Professional,Enterprise]/VC/Auxiliary/Build"
                "$ENV{ProgramFiles}/Microsoft Visual Studio/*/[Community,Professional,Enterprise]/VC/Auxiliary/Build")
                
            if(VS_INSTALLATIONS)
                list(GET VS_INSTALLATIONS 0 VS_DIR)
                if(EXISTS "${VS_DIR}/vcvarsall.bat")
                    # Create a temporary batch file to help find ml64
                    set(TEMP_BATCH_FILE "${CMAKE_BINARY_DIR}/find_ml64.bat")
                    file(WRITE ${TEMP_BATCH_FILE} "
                        @echo off
                        call \"${VS_DIR}/vcvarsall.bat\" x64 > nul
                        where ml64
                    ")
                    
                    execute_process(
                        COMMAND ${CMAKE_COMMAND} -E env "${TEMP_BATCH_FILE}"
                        OUTPUT_VARIABLE ML64_WHERE_OUTPUT
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                        ERROR_QUIET
                    )
                    
                    # Extract first line of where output
                    if(ML64_WHERE_OUTPUT)
                        string(REGEX REPLACE "\r?\n.*$" "" ML64_PATH "${ML64_WHERE_OUTPUT}")
                        if(EXISTS "${ML64_PATH}")
                            set(ML64 "${ML64_PATH}")
                        endif()
                    endif()
                endif()
            endif()
        endif()
    endif()
    
    # Check if we found ml64
    if(NOT ML64 OR NOT EXISTS "${ML64}")
        message(STATUS "ML64 not found. Trying alternative approach with AS_MAX_PORTABILITY.")
        # Fall back to using the portable approach that doesn't require assembly
        add_definitions(-DAS_MAX_PORTABILITY)
    else()
        message(STATUS "Found ML64: ${ML64}")
        
        # Set the asm file path
        set(ASM_FILE "${CMAKE_CURRENT_SOURCE_DIR}/include/angelscript/source/as_callfunc_x64_msvc_asm.asm")
        
        # Create a custom command to compile the assembly file
        set(ASM_OBJ "${CMAKE_CURRENT_BINARY_DIR}/as_callfunc_x64_msvc_asm.obj")
        add_custom_command(
            OUTPUT ${ASM_OBJ}
            COMMAND ${ML64} /c /nologo /Fo${ASM_OBJ} /W3 /Zi /Ta ${ASM_FILE}
            DEPENDS ${ASM_FILE}
            COMMENT "Assembling ${ASM_FILE}"
        )
        
        # Add the object file to your library
        target_sources(PrismEngine PRIVATE ${ASM_OBJ})
    endif()
endif()

# Add include directories
target_include_directories(PrismEngine 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/angelscript/include>
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
target_link_libraries(PrismEngine 
    PUBLIC 
        glfw
        daxa::daxa
        glm::glm
        fmt::fmt
        imgui::imgui
)

# Create resources directories
add_custom_command(TARGET PrismEngine PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders"
    COMMENT "Creating engine resource directories"
)