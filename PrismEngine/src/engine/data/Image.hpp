#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdexcept>
#include <string>

class Image
{
public:
    // Constructor - loads image from file
    explicit Image(const std::string &filename, int desired_channels = 0)
        : data_(nullptr), width_(0), height_(0), channels_(0)
    {
        data_ = stbi_load(filename.c_str(), &width_, &height_, &channels_, desired_channels);

        if (!data_)
        {
            throw std::runtime_error(std::string("Failed to load image: ") + stbi_failure_reason());
        }

        if (desired_channels != 0)
        {
            channels_ = desired_channels;
        }
    }

    // Move constructor
    Image(Image &&other) noexcept
        : data_(other.data_), width_(other.width_), height_(other.height_), channels_(other.channels_)
    {
        other.data_ = nullptr;
        other.width_ = 0;
        other.height_ = 0;
        other.channels_ = 0;
    }

    // Move assignment operator
    Image &operator=(Image &&other) noexcept
    {
        if (this != &other)
        {
            if (data_)
            {
                stbi_image_free(data_);
            }

            data_ = other.data_;
            width_ = other.width_;
            height_ = other.height_;
            channels_ = other.channels_;
            other.data_ = nullptr;
            other.width_ = 0;
            other.height_ = 0;
            other.channels_ = 0;
        }

        return *this;
    }

    // Destructor
    ~Image()
    {
        if (data_)
        {
            stbi_image_free(data_);
        }
    }

    // Delete copy constructor and assignment to prevent double-frees
    Image(const Image &) = delete;
    Image &operator=(const Image &) = delete;

    // Accessors
    int width() const { return width_; }

    int height() const { return height_; }

    int channels() const { return channels_; }

    // Get raw pixel data
    const unsigned char *data() const { return data_; }

    unsigned char *data() { return data_; }

    // Get pixel at position
    const unsigned char *pixel(int x, int y) const
    {
        if (x < 0 || x >= width_ || y < 0 || y >= height_)
        {
            throw std::out_of_range("Pixel coordinates out of bounds");
        }

        return data_ + (y * width_ + x) * channels_;
    }

    unsigned char *pixel(int x, int y)
    {
        if (x < 0 || x >= width_ || y < 0 || y >= height_)
        {
            throw std::out_of_range("Pixel coordinates out of bounds");
        }

        return data_ + (y * width_ + x) * channels_;
    }

    // Get size in bytes
    size_t size() const
    {
        return static_cast<size_t>(width_) * height_ * channels_;
    }

    // Check if image is valid
    bool valid() const
    {
        return data_ != nullptr;
    }

private:
    unsigned char *data_;
    int width_;
    int height_;
    int channels_;
};