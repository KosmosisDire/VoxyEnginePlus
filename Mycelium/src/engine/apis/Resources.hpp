#pragma once
#include <filesystem>
#include <string>

#include <vector> // Added for std::vector

namespace Resources
{
    // Function to get the full path to a resource file, searching if necessary
    std::filesystem::path GetResourcePath(const std::string& relativePath);

    // Function to read a resource file into a binary buffer (vector of chars)
    std::vector<char> ReadAsBinary(const std::string& relativePath);

    // Function to read a resource file into a string
    std::string ReadAsString(const std::string& relativePath);

} // namespace Resources
