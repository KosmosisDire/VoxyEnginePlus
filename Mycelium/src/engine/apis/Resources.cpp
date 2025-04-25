#include "Resources.hpp"
#include <fstream>
#include <vector>
#include <sstream>
#include <stdexcept> // For std::runtime_error

namespace Resources
{
    // Function to get the full path to a resource file, searching if necessary
    std::filesystem::path get_resource_path(const std::string& relativePath)
    {
        std::filesystem::path path = std::filesystem::current_path() / "resources" / relativePath;

        // check if the file exists, and do a search if it doesn't
        if (!std::filesystem::exists(path))
        {
            // Search recursively starting from the current path
            // This could be slow for large projects, consider optimizing if needed
            // e.g., by caching paths or searching only specific resource directories.
            bool found = false;

            for (const auto &entry : std::filesystem::recursive_directory_iterator(std::filesystem::current_path()))
            {
                // Check if it's a file and the filename matches
                if (entry.is_regular_file() && entry.path().filename() == relativePath)
                {
                    path = entry.path();
                    found = true;
                    break; // Found the first match
                }
            }

            // check if file exists after search and throw an error if it doesn't
            if (!found) // Use the 'found' flag instead of checking std::filesystem::exists(path) again
            {
                // Construct the error message carefully
                std::string errorMsg = "Resource not found: '" + relativePath + "'. Searched starting from: " + std::filesystem::current_path().string();
                throw std::runtime_error(errorMsg);
            }
        }

        return path;
    }

    // Core function to read a file into a binary buffer (vector of chars)
    std::vector<char> read_as_binary(const std::string& relativePath)
    {
        std::filesystem::path filePath = get_resource_path(relativePath); // Update call
        std::ifstream file(filePath, std::ios::binary | std::ios::ate); // Open at the end to get size

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open resource file: " + filePath.string());
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg); // Go back to the beginning

        std::vector<char> buffer(size);

        if (!file.read(buffer.data(), size))
        {
            throw std::runtime_error("Failed to read resource file: " + filePath.string());
        }

        return buffer;
    }

    // Function to read a file into a string
    std::string read_as_string(const std::string& relativePath)
    {
        std::filesystem::path filePath = get_resource_path(relativePath); // Update call
        std::ifstream file(filePath); // Open in text mode

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open resource file: " + filePath.string());
        }

        std::stringstream buffer;
        buffer << file.rdbuf(); // Read the whole file into the stringstream

        if (file.bad()) // Check for read errors
        {
            throw std::runtime_error("Failed to read resource file into string: " + filePath.string());
        }

        return buffer.str();
    }

} // namespace Resources
