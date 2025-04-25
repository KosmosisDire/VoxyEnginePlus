#pragma once
#include <filesystem>
#include <string>

#include <vector> // Added for std::vector

namespace Resources
{
    // Function to get the full path to a resource file, searching if necessary
    std::filesystem::path get_resource_path(const std::string& relativePath);

    // Function to read a resource file into a binary buffer (vector of chars)
    std::vector<char> read_as_binary(const std::string& relativePath);

    // Function to read a resource file into a string
    std::string read_as_string(const std::string& relativePath);

} // namespace Resources
