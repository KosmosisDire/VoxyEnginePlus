// scene_object.cppm
export module scene_object;

import object;
import scriptable_object;
import component;
import transform; // Needs concrete Transform

import <vector>;
import <memory>;
import <stdexcept>;
import <algorithm>;
import <typeinfo>;
import <type_traits>;

// Math includes likely needed for delegated methods if not using PIMPL
#include <engine/data/Matrix4x4.hpp>
#include <engine/data/Vector3.hpp>
#include <engine/data/Quaternion.hpp>

// Define Component::get_transform now that SceneObject and Transform are known
// This could go in component.cppm if SceneObject/Transform are fully imported there,
// or here, or a separate utility module. Placing it here for now.
inline std::shared_ptr<Transform> Component::get_transform() const
{
    if (auto owner = get_scene_object())
    {
        return owner->get_transform(); // Call SceneObject's getter
    }

    return nullptr;
}


export class SceneObject : public ScriptableObject
{
        // No more direct hierarchy members (m_parent, m_children)

    private:
        // Component management
        std::vector<std::shared_ptr<Component>> m_components;
        std::shared_ptr<Transform> m_transform; // Cached pointer to the Transform component

        // Helper to add component
        void add_component_internal(const std::shared_ptr<Component>& comp)
        {
            try
            {
                comp->set_owner(std::dynamic_pointer_cast<SceneObject>(shared_from_this()));
            }
            catch (const std::bad_weak_ptr&)
            {
                throw std::runtime_error("SceneObject must be managed by std::shared_ptr to add components.");
            }

            m_components.push_back(comp);
            comp->on_attach();
        }

        // Called only by constructor
        void initialize_transform()
        {
            m_transform = Object::create<Transform>();
            add_component_internal(m_transform);
        }

    public:
        // Constructors
        SceneObject(std::string name = "SceneObject")
            : ScriptableObject(std::move(name))
        {
            initialize_transform();
        }

        SceneObject(const GUID& guid, std::string name)
            : ScriptableObject(guid, std::move(name))
        {
            initialize_transform();
        }

        // Destructor
        ~SceneObject() override
        {
            // SceneObject destruction triggers Transform destruction (as it's a component owned via shared_ptr).
            // The Transform destructor (~Transform) now handles hierarchy detachment (detaching from parent, orphaning children).

            // Call detach on *other* components BEFORE transform potentially gets destroyed
            for (const auto& comp : m_components)
            {
                if (comp && comp != m_transform)   // Don't double-detach transform
                {
                    comp->on_detach();
                    comp->set_owner(nullptr);
                }
            }

            // Transform's on_detach will be called implicitly when m_components is cleared if it exists
            // or when m_transform shared_ptr goes out of scope.
            m_components.clear();
            // m_transform will be released here automatically.
        }

        // --- Lifecycle Methods (Forward to Components) ---
        // ... (on_create, on_destroy, on_update, on_render remain the same, iterating m_components) ...
        void on_create() override { /* ... call comp->on_start() ... */ }

        void on_destroy() override { /* ... base call if needed ... */ }

        void on_update(float deltaTime) override { /* ... call comp->on_update() ... */ }

        void on_render() override { /* ... call comp->on_render() ... */ }


        // --- Component Management (Mostly Unchanged) ---
        // ... (add_component, get_component<T>, get_components<T>, remove_component<T> remain the same) ...
        template <typename T, typename... Args>
        std::shared_ptr<T> add_component(Args&&... args) { /* ... same ... */ }

        template <typename T>
        std::shared_ptr<T> get_component() const { /* ... same ... */ }

        template<typename T>
        std::vector<std::shared_ptr<T>> get_components() const { /* ... same ... */ }
        template <typename T>
        bool remove_component() { /* ... same, ensuring Transform isn't removed ... */ }


        // --- Transform Access and Delegation ---

        // Direct access to the Transform component
        std::shared_ptr<Transform> get_transform() const
        {
            return m_transform;
        }

        // --- Hierarchy Delegation ---

        // Get parent SceneObject by going through transforms
        std::shared_ptr<SceneObject> get_parent() const
        {
            if (!m_transform) return nullptr;

            std::shared_ptr<Transform> parentTransform = m_transform->get_parent();
            return parentTransform ? parentTransform->get_scene_object() : nullptr;
        }

        // Set parent by setting the parent of the Transform component
        // Takes another SceneObject whose transform will become the parent.
        void set_parent(const std::shared_ptr<SceneObject>& newParentSO)
        {
            if (!m_transform) return; // Should not happen

            Transform* newParentTransformPtr = newParentSO ? newParentSO->get_transform().get() : nullptr;
            m_transform->set_parent(newParentTransformPtr);
        }

        // Get child SceneObject by index (less common, requires iterating transform children)
        std::shared_ptr<SceneObject> get_child(int index) const
        {
            if (!m_transform) return nullptr;

            Transform* childTransform = m_transform->get_child(index);
            // Need to get the SceneObject owner from the child Transform
            return childTransform ? childTransform->get_scene_object() : nullptr;
        }

        int get_child_count() const
        {
            return m_transform ? m_transform->get_child_count() : 0;
        }

        // Find child SceneObject by name (delegates to Transform potentially, or searches children SOs)
        std::shared_ptr<SceneObject> find(const std::string& name) const
        {
            if (!m_transform) return nullptr;

            for (int i = 0; i < m_transform->get_child_count(); ++i)
            {
                Transform* childT = m_transform->get_child(i);

                if (childT)
                {
                    if(auto childSO = childT->get_scene_object())
                    {
                        if (childSO->get_name() == name)
                        {
                            return childSO;
                        }

                        // Optional: Recursive find
                        // if (auto found = childSO->find(name)) { return found; }
                    }
                }
            }

            return nullptr;
        }


        // --- World Space Delegation ---
        // Delegate all world-space operations directly to the Transform component

        Matrix4x4 get_world_matrix() const { return m_transform ? m_transform->get_world_matrix() : Matrix4x4::identity(); }

        Matrix4x4 get_world_to_local_matrix() const { return m_transform ? m_transform->get_world_to_local_matrix() : Matrix4x4::identity(); }

        Vector3 get_world_position() const { return m_transform ? m_transform->get_world_position() : Vector3::zero(); }

        Quaternion get_world_rotation() const { return m_transform ? m_transform->get_world_rotation() : Quaternion::identity(); }

        Vector3 get_world_scale() const { return m_transform ? m_transform->get_world_scale() : Vector3::one(); }

        void set_world_position(const Vector3& worldPos) { if(m_transform) m_transform->set_world_position(worldPos); }

        void set_world_rotation(const Quaternion& worldRot) { if(m_transform) m_transform->set_world_rotation(worldRot); }

        void look_at(const Vector3& worldTarget, const Vector3& worldUp = Vector3::up()) { if(m_transform) m_transform->look_at(worldTarget, worldUp); }

        Vector3 transform_point_to_world(const Vector3& localPoint) const { return m_transform ? m_transform->transform_point(localPoint) : localPoint; }

        Vector3 transform_direction_to_world(const Vector3& localDirection) const { return m_transform ? m_transform->transform_direction(localDirection) : localDirection; }

        Vector3 transform_point_to_local(const Vector3& worldPoint) const { return m_transform ? m_transform->inverse_transform_point(worldPoint) : worldPoint; }

        Vector3 transform_direction_to_local(const Vector3& worldDirection) const { return m_transform ? m_transform->inverse_transform_direction(worldDirection) : worldDirection; }
};