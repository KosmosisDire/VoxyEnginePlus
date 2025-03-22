#pragma once

#include <span>
#include <cstddef>
#include <functional>

// Helper for std::span size_bytes()
template <typename T>
inline size_t span_bytes(const std::span<const T>& span)
{
    return span.size() * sizeof(T);
}

// Custom hash function for spans
template <typename T>
struct SpanHash
{
    size_t operator()(const std::span<const T>& span) const
    {
        // FNV-1a hash algorithm
        size_t hash = 14695981039346656037ULL; // FNV offset basis
        const size_t prime = 1099511628211ULL;  // FNV prime

        // Hash each byte of data
        const uint8_t* data = reinterpret_cast<const uint8_t*>(span.data());
        size_t byte_size = span.size() * sizeof(T);

        for (size_t i = 0; i < byte_size; ++i)
        {
            hash ^= static_cast<size_t>(data[i]);
            hash *= prime;
        }

        return hash;
    }
};

// Specialize for types with existing hash function
template <>
struct SpanHash<uint8_t>
{
    size_t operator()(const std::span<const uint8_t>& span) const
    {
        // Use std::hash for individual elements and combine
        size_t hash = 0;

        for (const auto& element : span)
        {
            hash ^= std::hash<uint8_t> {}(element) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }

        return hash;
    }
};

// Custom equality comparison for spans
template <typename T>
struct SpanEqual
{
    bool operator()(const std::span<const T>& lhs, const std::span<const T>& rhs) const
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        for (size_t i = 0; i < lhs.size(); ++i)
        {
            if (!(lhs[i] == rhs[i]))
            {
                return false;
            }
        }

        return true;
    }
};