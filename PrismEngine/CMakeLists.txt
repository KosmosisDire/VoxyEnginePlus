file(GLOB_RECURSE APP_SRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

# Use PROJECT_NAME consistently
add_executable(${PROJECT_NAME} ${APP_SRC})

# Set RPATH properties for the executable
set_target_properties(${PROJECT_NAME} PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib:${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/lib:${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/debug/lib"
)

include_directories("${PROJECT_BINARY_DIR}/include")
# Add include directories for the application
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/Mycelium/src
        ${PROJECT_BINARY_DIR}/include
)

# Link against the engine library
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        Mycelium
)

# Function to handle resource deployment based on platform and build type
function(add_resource_folder TARGET SOURCE_PATH DEST_REL_PATH)
    set(FULL_DEST_PATH "$<TARGET_FILE_DIR:${TARGET}>/${DEST_REL_PATH}")
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(WIN32)
            # For Windows Debug mode, add to the script for hardlinking
            set_property(TARGET ${TARGET} APPEND PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "--resource-path ${SOURCE_PATH} --dest-path ${DEST_REL_PATH}")
        else()
            # For Linux/macOS Debug mode, use symlinks
            add_custom_command(TARGET ${TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${TARGET}>"
                COMMAND ${CMAKE_COMMAND} -E create_symlink
                    "${SOURCE_PATH}"
                    "${FULL_DEST_PATH}"
                COMMENT "Creating symlink for ${SOURCE_PATH} -> ${FULL_DEST_PATH}"
            )
        endif()
    else()
        # For Release mode, copy directories
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${FULL_DEST_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${SOURCE_PATH}"
                "${FULL_DEST_PATH}"
            COMMENT "Copying ${SOURCE_PATH} to ${FULL_DEST_PATH}"
        )
    endif()
endfunction()

# For Windows Debug mode, create a PowerShell script to handle all hard linking
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Create the PowerShell script for resource hard links
    file(WRITE "${CMAKE_BINARY_DIR}/create_hardlinks.ps1" 
         "Write-Host \"Creating hard links for hot reloading...\"\n"
         "\n"
         "# Create destination base directory\n"
         "New-Item -Path \"${CMAKE_BINARY_DIR}/PrismEngine/Debug\" -ItemType Directory -Force | Out-Null\n"
         "\n"
         "# Function to create hard links preserving directory structure\n"
         "function Create-HardLinks($sourcePath, $targetBasePath, $relativeToPath) {\n"
         "    # Create the initial directory\n"
         "    New-Item -Path $targetBasePath -ItemType Directory -Force | Out-Null\n"
         "    \n"
         "    # Create directory structure\n"
         "    Get-ChildItem -Path $sourcePath -Recurse -Directory | ForEach-Object {\n"
         "        $relPath = $_.FullName.Substring($relativeToPath.Length)\n"
         "        New-Item -Path \"$targetBasePath$relPath\" -ItemType Directory -Force | Out-Null\n"
         "    }\n"
         "    \n"
         "    # Create hard links for all files\n"
         "    Get-ChildItem -Path $sourcePath -Recurse -File | ForEach-Object {\n"
         "        $sourceFile = $_.FullName\n"
         "        $relativePath = $sourceFile.Substring($relativeToPath.Length)\n"
         "        $targetPath = Join-Path -Path $targetBasePath -ChildPath $relativePath\n"
         "        \n"
         "        try {\n"
         "            New-Item -ItemType HardLink -Path $targetPath -Target $sourceFile -Force | Out-Null\n"
         "            Write-Host \"Created hard link: $targetPath -> $sourceFile\"\n"
         "        } catch {\n"
         "            Write-Warning \"Failed to create hard link for $sourceFile. Copying instead.\"\n"
         "            Copy-Item -Path $sourceFile -Destination $targetPath -Force\n"
         "        }\n"
         "    }\n"
         "}\n"
    )
    
    # Add resource folders
    file(APPEND "${CMAKE_BINARY_DIR}/create_hardlinks.ps1"
        "# Process engine resources\n"
        "Create-HardLinks \"${CMAKE_SOURCE_DIR}/Mycelium/resources\" \"${CMAKE_BINARY_DIR}/PrismEngine/Debug/resources\" \"${CMAKE_SOURCE_DIR}/Mycelium/resources\"\n"
        "\n"
        "# Process shader resources\n"
        "Create-HardLinks \"${CMAKE_SOURCE_DIR}/PrismEngine/src/vox/shaders\" \"${CMAKE_BINARY_DIR}/PrismEngine/Debug/resources/shaders\" \"${CMAKE_SOURCE_DIR}/PrismEngine/src/vox/shaders\"\n"
        "\n"
        "# Process application resources\n"
        "Create-HardLinks \"${CMAKE_SOURCE_DIR}/PrismEngine/resources\" \"${CMAKE_BINARY_DIR}/PrismEngine/Debug/resources\" \"${CMAKE_SOURCE_DIR}/PrismEngine/resources\"\n"
        "\n"
        "Write-Host \"Hard links created successfully!\"\n"
        "exit 0\n"  # Ensure script always exits with success
    )
    
    # Run the script as a single post-build command
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND powershell.exe -ExecutionPolicy Bypass -NoProfile -NonInteractive -File "${CMAKE_BINARY_DIR}/create_hardlinks.ps1"
        VERBATIM
        COMMENT "Setting up resource hard links for hot reloading"
    )
endif()

# Add resource folders
add_resource_folder(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/Mycelium/resources" "resources")
add_resource_folder(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/PrismEngine/src/vox/shaders" "resources/shaders")
add_resource_folder(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/PrismEngine/src/resources" "resources")

# DLL handling with safer command
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime DLLs"
    )
endif()