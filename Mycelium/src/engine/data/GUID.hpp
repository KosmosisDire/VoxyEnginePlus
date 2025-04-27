#include <cstdint>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <array>

class GUID
{
    private:
        // The actual GUID data - 16 bytes following UUID standard
        std::array<uint8_t, 16> data;

        // String representation cache
        mutable std::string stringRepresentation;

        // Generate random bytes for the GUID
        void generate()
        {
            // Use a good random number generator
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<uint32_t> dis(0, 255);

            // Fill the array with random bytes
            for (int i = 0; i < 16; ++i)
            {
                data[i] = static_cast<uint8_t>(dis(gen));
            }

            // Set the version (4 = random UUID) and variant bits according to RFC 4122
            data[6] = (data[6] & 0x0F) | 0x40;  // Version 4
            data[8] = (data[8] & 0x3F) | 0x80;  // Variant 1
        }

        // Generate the string representation of the GUID
        void update_string_representation() const
        {
            std::stringstream ss;
            ss << std::hex << std::setfill('0');

            // Format: 8-4-4-4-12 (rfc4122)
            for (int i = 0; i < 16; ++i)
            {
                if (i == 4 || i == 6 || i == 8 || i == 10)
                {
                    ss << '-';
                }

                ss << std::setw(2) << static_cast<int>(data[i]);
            }

            stringRepresentation = ss.str();
        }

    public:
        // Constructor - automatically generates a new GUID
        GUID()
        {
            generate();
        }

        // Get the string representation (lazily generated)
        const std::string& to_string() const
        {
            if (stringRepresentation.empty())
            {
                update_string_representation();
            }

            return stringRepresentation;
        }

        // Check if two GUIDs are equal
        bool operator==(const GUID& other) const
        {
            return data == other.data;
        }

        // Check if two GUIDs are not equal
        bool operator!=(const GUID& other) const
        {
            return !(*this == other);
        }

        // Get raw data access (read-only)
        const std::array<uint8_t, 16>& get_data() const
        {
            return data;
        }
};