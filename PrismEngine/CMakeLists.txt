file(GLOB_RECURSE APP_SRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

# Define the executable target
add_executable(${PROJECT_NAME} ${APP_SRC}) # Use PROJECT_NAME from top-level
 
# Set RPATH properties (keep your existing logic if needed for Linux/macOS)
if(UNIX AND NOT APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib:${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/lib:${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/debug/lib"
    )
endif()

# Define Include Directories for the executable
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_BINARY_DIR}/include # For generated headers like project_paths.h
        # ANTLR include dirs should be inherited via Mycelium PUBLIC linkage
)

# Link executable against libraries
target_link_libraries(${PROJECT_NAME}
    PUBLIC # Changed from PRIVATE to inherit usage requirements
        Mycelium # Link the engine library (which now includes ANTLR sources)
)

# --- Resource Handling ---
# Define resource source locations relative to CMAKE_SOURCE_DIR
set(MYCELIUM_RESOURCE_DIR ${CMAKE_SOURCE_DIR}/Mycelium/resources)
set(PRISM_SHADER_DIR ${CMAKE_SOURCE_DIR}/PrismEngine/src/vox/shaders) # Check this path
set(PRISM_RESOURCE_DIR ${CMAKE_SOURCE_DIR}/PrismEngine/resources) # Check this path

# Define the base destination directory relative to the executable
set(DEST_RESOURCE_BASE_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources")

# Windows Debug: Use PowerShell script for hard links
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PS_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/setup_debug_resources.ps1") # Use CURRENT_BINARY_DIR
    set(TARGET_EXE_DIR_PS "$($(<TARGET_FILE_DIR:${PROJECT_NAME}>) -replace '\\', '\\\\')") # Escaped path for PS

    # PowerShell Script Content (Improved Structure)
    file(WRITE ${PS_SCRIPT} "
param(
    [string]\$TargetExeDir,
    [string[]]\$SourceDirs,
    [string[]]\$SourceBasePaths,
    [string]\$DestBaseDir
)

Write-Host \"Setting up debug resources via hard links...\"

# Ensure base destination exists
\$null = New-Item -Path \$DestBaseDir -ItemType Directory -Force

# Function to create hard links preserving structure
function Create-HardLinks(\$sourcePath, \$targetPath, \$baseSourcePath) {
    Write-Host \"Processing: \$sourcePath -> \$targetPath\"
    if (-not (Test-Path \$sourcePath)) {
        Write-Warning \"Source path does not exist: \$sourcePath\"
        return
    }
    \$null = New-Item -Path \$targetPath -ItemType Directory -Force

    Get-ChildItem -Path \$sourcePath -Recurse -Directory | ForEach-Object {
        \$relPath = \$_.FullName.Substring(\$baseSourcePath.Length)
        \$null = New-Item -Path (Join-Path \$targetPath \$relPath) -ItemType Directory -Force
    }

    Get-ChildItem -Path \$sourcePath -Recurse -File | ForEach-Object {
        \$srcFile = \$_.FullName
        \$relPath = \$srcFile.Substring(\$baseSourcePath.Length)
        \$destFile = Join-Path \$targetPath \$relPath
        \$destDir = Split-Path \$destFile -Parent
        if (-not (Test-Path \$destDir)) {
            \$null = New-Item -Path \$destDir -ItemType Directory -Force
        }
        try {
            # Remove existing item first if it exists (might be file or link)
            if (Test-Path \$destFile) { Remove-Item \$destFile -Force }
            New-Item -ItemType HardLink -Path \$destFile -Target \$srcFile -Force | Out-Null
            # Write-Host \"  Linked: \$destFile -> \$srcFile\"
        } catch {
            Write-Warning \"  Failed hard link for \$srcFile. Error: \$(\$_.Exception.Message)\"
        }
    }
}

# Process each resource directory
for (\$i = 0; \$i -lt \$SourceDirs.Length; \$i++) {
    \$source = \$SourceDirs[\$i]
    \$base = \$SourceBasePaths[\$i]
    \$destRel = (Split-Path \$source -Leaf) # Use last component as dest subfolder
    \$dest = Join-Path \$DestBaseDir \$destRel

    # Special case for shaders going into resources/shaders
    if (\$source -like '*vox/shaders') {
        \$dest = Join-Path \$DestBaseDir 'shaders'
    } elseif (\$source -like '*Mycelium/resources') {
         \$dest = \$DestBaseDir # Mycelium resources go directly into dest base
    } elseif (\$source -like '*PrismEngine/resources') {
         \$dest = \$DestBaseDir # PrismEngine resources go directly into dest base
    }

    Create-HardLinks -sourcePath \$source -targetPath \$dest -baseSourcePath \$base
}

Write-Host \"Debug resource setup finished.\"
exit 0
") # End of file(WRITE)

    # Add the single post-build command to execute the script
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND powershell.exe -ExecutionPolicy Bypass -NoProfile -NonInteractive -File "${PS_SCRIPT}" -TargetExeDir "$<TARGET_FILE_DIR:${PROJECT_NAME}>" -SourceDirs "${MYCELIUM_RESOURCE_DIR};${PRISM_SHADER_DIR};${PRISM_RESOURCE_DIR}" -SourceBasePaths "${MYCELIUM_RESOURCE_DIR};${PRISM_SHADER_DIR};${PRISM_RESOURCE_DIR}" -DestBaseDir "${DEST_RESOURCE_BASE_DIR}"
        VERBATIM
        COMMENT "Setting up resource hard links via PowerShell"
    )

# Non-Windows Debug (Symlinks) / Release (Copy)
else()
    # Function to add symlink/copy command
    function(add_resource_deployment TARGET SOURCE_PATH DEST_REL_PATH)
        set(FULL_DEST_PATH "$<TARGET_FILE_DIR:${TARGET}>/${DEST_REL_PATH}")
        set(ACTION_VERB "Copying")
        set(CMAKE_ACTION "-E copy_directory_if_different") # Safer copy
        if(NOT CMAKE_BUILD_TYPE STREQUAL "Release") # Assume Debug/RelWithDebInfo etc. use links
           if(UNIX OR APPLE)
                set(ACTION_VERB "Symlinking")
                set(CMAKE_ACTION "-E create_symlink")
           endif()
        endif()

        # Ensure source exists before adding command
        if(EXISTS "${SOURCE_PATH}")
            add_custom_command(TARGET ${TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${FULL_DEST_PATH}"
                COMMAND ${CMAKE_COMMAND} ${CMAKE_ACTION} "${SOURCE_PATH}" "${FULL_DEST_PATH}"
                COMMENT "${ACTION_VERB} ${SOURCE_PATH} -> ${FULL_DEST_PATH}"
                VERBATIM
            )
        else()
            message(WARNING "Resource source path not found, skipping deployment: ${SOURCE_PATH}")
        endif()
    endfunction()

    # Deploy resources using the function
    add_resource_deployment(${PROJECT_NAME} "${MYCELIUM_RESOURCE_DIR}" "resources") # Mycelium resources into 'resources' subdir
    add_resource_deployment(${PROJECT_NAME} "${PRISM_SHADER_DIR}" "resources/shaders") # Shaders into 'resources/shaders'
    add_resource_deployment(${PROJECT_NAME} "${PRISM_RESOURCE_DIR}" "resources") # PrismEngine resources into 'resources' subdir

endif()

# DLL Handling (Requires CMake 3.21+)
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS # Important for generator expression list
        COMMENT "Copying runtime DLLs"
        VERBATIM
    )
endif()
