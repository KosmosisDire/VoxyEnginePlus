#include <string>
#include <vector>
#include <iostream>
#include <type_traits>
#include <utility>

struct Image { std::string name; };

// Define a configuration struct with desired options
struct ReadConfig
{
    std::string config1 = "";
    std::vector<int> config2 = {};
    enum class ViewType { _1D, _2D, _3D } config3 = ViewType::_2D;

    // Add any other config options here
};

// Tag type to identify config parameters during resolution
template <typename T>
struct ConfigParameter
{
    T value;
    explicit ConfigParameter(T v) : value(std::forward<T>(v)) {}
};

// Helper to check if a type is our config struct
template <typename T>
struct is_config : std::false_type {};

template <>
struct is_config<ReadConfig> : std::true_type {};

// Helper to check if a type is wrapped in our ConfigParameter
template <typename T>
struct is_config_parameter : std::false_type {};

template <typename T>
struct is_config_parameter<ConfigParameter<T>> : std::true_type {};

// Primary template - version with regular parameters
template <typename... Args>
void read(Args&&... args)
{
    // Implementation for when no config is provided
    process_images(std::forward<Args>(args)...);
}

// Specialization for when a ReadConfig is the last parameter
template <typename... Args>
void read(Args&&... args, ReadConfig config)
{
    // Process images with configuration
    process_images_with_config(std::forward<Args>(args)..., config);
}

// Helper functions to process the images
template <typename... Images>
void process_images(Images&&... images)
{
    // Default processing without config
    std::cout << "Processing " << sizeof...(images) << " images with default config" << std::endl;
    // Implementation...
}

template <typename... Images>
void process_images_with_config(Images&&... images, const ReadConfig& config)
{
    // Process with specific config
    std::cout << "Processing " << sizeof...(images) << " images with custom config:" << std::endl;
    std::cout << "  config1: " << config.config1 << std::endl;
    std::cout << "  config2: {";

    for (const auto& val : config.config2)
    {
        std::cout << " " << val;
    }

    std::cout << " }" << std::endl;
    std::cout << "  config3: " << static_cast<int>(config.config3) << std::endl;
    // Implementation...
}