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

# Define resource source directories
set(RESOURCE_SOURCE_DIRS
    "${CMAKE_SOURCE_DIR}/Mycelium/resources"
    "${CMAKE_SOURCE_DIR}/PrismEngine/resources"
)

# Windows Debug: Use PowerShell script for hard links
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(POWERSHELL_SCRIPT_PATH "${CMAKE_CURRENT_BINARY_DIR}/create_hardlinks.ps1")

    # Base script content with function definition
    file(WRITE "${POWERSHELL_SCRIPT_PATH}"
        "param(\n"
        "    [string]\$TargetBuildDir # Passed from add_custom_command\n"
        ")\n\n"
        "Write-Host \"Creating hard links for hot reloading into: \$TargetBuildDir\"\n\n"
        "# Ensure base destination directory exists\n"
        "\$destBase = Join-Path -Path \$TargetBuildDir -ChildPath 'resources'\n"
        "if (-not (Test-Path \$destBase)) {\n"
        "    New-Item -Path \$destBase -ItemType Directory -Force | Out-Null\n"
        "}\n\n"
        "# Function to create hard links preserving directory structure\n"
        "function Create-HardLinks(\$sourcePath, \$targetBasePath, \$relativeToPath) {\n"
        "    Write-Host \"Processing source: \$sourcePath -> \$targetBasePath\"\n"
        "    # Create directory structure first\n"
        "    Get-ChildItem -Path \$sourcePath -Recurse -Directory | ForEach-Object {\n"
        "        \$relPath = \$_.FullName.Substring(\$relativeToPath.Length)\n"
        "        \$fullTargetPath = Join-Path -Path \$targetBasePath -ChildPath \$relPath\n"
        "        if (-not (Test-Path \$fullTargetPath)) {\n"
        "             New-Item -Path \$fullTargetPath -ItemType Directory -Force | Out-Null\n"
        "        }\n"
        "    }\n\n"
        "    # Create hard links for all files\n"
        "    Get-ChildItem -Path \$sourcePath -Recurse -File | ForEach-Object {\n"
        "        \$sourceFile = \$_.FullName\n"
        "        \$relativePath = \$sourceFile.Substring(\$relativeToPath.Length)\n"
        "        \$targetPath = Join-Path -Path \$targetBasePath -ChildPath \$relativePath\n\n"
        "        # Remove existing item at target path if it exists\n"
        "        if (Test-Path \$targetPath) {\n"
        "            Remove-Item \$targetPath -Force -Recurse\n"
        "        }\n\n"
        "        try {\n"
        "            New-Item -ItemType HardLink -Path \$targetPath -Target \$sourceFile -Force | Out-Null\n"
        "            # Write-Host \"  Link: \$targetPath -> \$sourceFile\"\n"
        "        } catch {\n"
        "            Write-Warning \"  Failed hard link for \$sourceFile. Error: \$(\$_.Exception.Message)\"\n"
        "            # Optional: Copy as fallback? \n"
        "            # Write-Host \"  Copying instead: \$sourceFile -> \$targetPath\"\n"
        "            # Copy-Item -Path \$sourceFile -Destination \$targetPath -Force\n"
        "        }\n"
        "    }\n"
        "}\n\n"
    )

    # Append calls for each resource directory
    foreach(SOURCE_DIR ${RESOURCE_SOURCE_DIRS})
        # Need to escape backslashes in paths for PowerShell strings
        string(REPLACE "\\" "\\\\" SOURCE_DIR_ESC "${SOURCE_DIR}")
        file(APPEND "${POWERSHELL_SCRIPT_PATH}"
            "Create-HardLinks \"${SOURCE_DIR_ESC}\" \"\$destBase\" \"${SOURCE_DIR_ESC}\"\n"
        )
    endforeach()

    file(APPEND "${POWERSHELL_SCRIPT_PATH}"
        "Write-Host \"Resource hard linking complete.\"\n"
        "exit 0\n"
    )

    # Add the post-build command to execute the script
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND powershell.exe -ExecutionPolicy Bypass -NoProfile -NonInteractive
                -File "${POWERSHELL_SCRIPT_PATH}"
                -TargetBuildDir "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        VERBATIM
        COMMENT "Setting up resource hard links via PowerShell"
    )

# Other Builds (Release all platforms, Debug Linux/macOS)
else()
    foreach(SOURCE_DIR ${RESOURCE_SOURCE_DIRS})
        # Destination is base resources directory next to executable
        set(FULL_DEST_PATH "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources")

        if(CMAKE_BUILD_TYPE STREQUAL "Debug") # Linux/macOS Debug
            set(ACTION_VERB "Symlinking")
            set(CMAKE_COMMAND_ACTION create_symlink)
            set(SOURCE_ARG "${SOURCE_DIR}") # Symlink the source directory itself
            # Need the name of the source directory for the destination link name
            get_filename_component(DIR_NAME "${SOURCE_DIR}" NAME)
            set(DEST_ARG "${FULL_DEST_PATH}/${DIR_NAME}")
        else() # Release (all platforms)
            set(ACTION_VERB "Copying")
            set(CMAKE_COMMAND_ACTION copy_directory)
            set(SOURCE_ARG "${SOURCE_DIR}")
            set(DEST_ARG "${FULL_DEST_PATH}") # Copy contents into resources/
        endif()

        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${FULL_DEST_PATH}" # Ensure base dir exists
            COMMAND ${CMAKE_COMMAND} -E ${CMAKE_COMMAND_ACTION}
                "${SOURCE_ARG}"
                "${DEST_ARG}"
            COMMENT "${ACTION_VERB} resources: ${SOURCE_DIR} -> ${DEST_ARG}"
            VERBATIM
        )
    endforeach()
endif()


# --- DLL Handling (Requires CMake 3.21+) ---
# (Keep existing DLL handling logic)
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS # Important for generator expression list
        COMMENT "Copying runtime DLLs"
        VERBATIM
    )
endif()
