// component.cppm
export module component;

import object;
import <memory>; // For weak_ptr, shared_ptr

// Forward declare SceneObject to avoid circular module dependency if possible,
// but weak_ptr might require the full definition depending on standard library implementation.
// If compilation fails here, you might need to import scene_object fully.
// export class SceneObject;
// For simplicity and common compiler behavior, let's import:
import scene_object; // NOTE: Creates a dependency cycle (SceneObject -> Component -> SceneObject)
// which modules handle, but careful design is needed.
// A potentially cleaner approach uses interfaces or PIMPL.


export class Component : public Object
{
        friend class SceneObject; // Allow SceneObject to set m_owner

    private:
        // Non-owning pointer back to the SceneObject this component is attached to.
        // Use weak_ptr to avoid reference cycles.
        std::weak_ptr<SceneObject> m_owner;

    protected:
        // Constructor accessible only to derived classes and SceneObject (via friend)
        using Object::Object; // Inherit Object constructors

        // Allow SceneObject to set the owner after creation
        void set_owner(std::shared_ptr<SceneObject> owner)
        {
            m_owner = owner;
        }

    public:
        ~Component() override = default; // Virtual destructor

        // Get the SceneObject this component is attached to.
        // Returns nullptr if the SceneObject has been destroyed.
        std::shared_ptr<SceneObject> get_scene_object() const
        {
            return m_owner.lock();
        }

        // --- Optional Component Lifecycle Methods ---
        // Called when the component is added to a SceneObject
        virtual void on_attach() {}

        // Called just before the component is removed or the SceneObject is destroyed
        virtual void on_detach() {}

        // Called once before the first update frame
        virtual void on_start() {}

        // Called every frame
        virtual void on_update(float /*deltaTime*/) {}

        // Called for rendering (if applicable)
        virtual void on_render() {}

        // Example: Accessing the Transform is common
        // (We'll define Transform later, forward declare or include necessary headers)
        // class Transform; // Forward declaration
        // std::shared_ptr<Transform> get_transform() const; // Implementation needs SceneObject
};

// Implementation for get_transform requires SceneObject definition
// We can put it here *if* scene_object module is fully imported above.
#include "SceneObject.hpp" // Or import scene_object; if SceneObject has get_transform
#include "Transform.hpp"   // Or import transform;

inline std::shared_ptr<Transform> Component::get_transform() const
{
    if (auto owner = get_scene_object())
    {
        return owner->get_transform(); // Assumes SceneObject has get_transform()
    }

    return nullptr;
}