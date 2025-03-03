#pragma once
#include <filesystem>
#include <string>

namespace Resources
{
    inline std::filesystem::path GetResourcePath(const std::string relativePath)
    {
        std::filesystem::path path = std::filesystem::current_path() / "resources" / relativePath;

        // check if the file exists, and do a search if it doesn't
        if (!std::filesystem::exists(path))
        {
            for (const auto &entry : std::filesystem::recursive_directory_iterator(std::filesystem::current_path()))
            {
                if (entry.path().filename() == relativePath)
                {
                    path = entry.path();
                    break;
                }
            }

            
            // check if file exists and throw an error if it doesn't
            if (!std::filesystem::exists(path))
            {
                throw std::runtime_error("Resource not found: " + path.string());
            }
        }

        return path;
    }
}
