// object.cppm
export module object;

import <string>;
import <memory>; // For shared_ptr, enable_shared_from_this
import <type_traits>; // For is_base_of
import <utility>; // For std::forward

// Assuming GUID is available via include. If it's a module: import engine.data.guid;
#include <engine/data/GUID.hpp>

export class Object : public std::enable_shared_from_this<Object>
{
    private:
        GUID m_id;
        std::string m_name;

    public:
        // Constructors
        Object(const GUID& guid, std::string objName)
            : m_id(guid), m_name(std::move(objName)) {}

        Object(std::string objName)
            : m_id(), m_name(std::move(objName)) {} // Generate new GUID

        Object()
            : m_id(), m_name("Object") {} // Generate new GUID

        // Virtual destructor is crucial for base classes with virtual functions
        virtual ~Object() = default;

        // Prevent copying/moving for simplicity with shared_ptr ownership
        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;
        Object(Object&&) = delete;
        Object& operator=(Object&&) = delete;

        // Accessors
        const GUID& get_id() const noexcept { return m_id; }

        const std::string& get_name() const noexcept { return m_name; }

        void set_name(std::string newName) { m_name = std::move(newName); }

        // Utility
        virtual std::string to_string() const { return m_name; }

        // Comparison based on ID
        bool operator==(const Object& other) const { return m_id == other.m_id; }

        bool operator!=(const Object& other) const { return !(*this == other); }

        // Static factory methods using std::shared_ptr
        template <typename T, typename... Args>
        static std::shared_ptr<T> instantiate(Args&&... args)
        {
            static_assert(std::is_base_of_v<Object, T>, "T must inherit from Object");
            // Use make_shared for efficiency and exception safety
            return std::make_shared<T>(std::forward<Args>(args)...);
        }

        // Simplified factory for default construction
        template <typename T>
        static std::shared_ptr<T> create()
        {
            static_assert(std::is_base_of_v<Object, T>, "T must inherit from Object");
            static_assert(std::is_default_constructible_v<T>, "T must be default constructible for create()");
            return std::make_shared<T>();
        }

        // Helper to get shared_ptr from 'this' (requires derived class to also inherit enable_shared_from_this)
        // Note: Only safe to call when the object is already managed by a shared_ptr
        template <typename Derived>
        std::shared_ptr<Derived> shared_this()
        {
            static_assert(std::is_base_of_v<Object, Derived>, "Derived must inherit from Object");
            return std::dynamic_pointer_cast<Derived>(shared_from_this());
        }

        template <typename Derived>
        std::shared_ptr<const Derived> shared_this() const
        {
            static_assert(std::is_base_of_v<Object, Derived>, "Derived must inherit from Object");
            return std::dynamic_pointer_cast<const Derived>(shared_from_this());
        }
};