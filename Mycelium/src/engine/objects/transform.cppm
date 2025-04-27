// transform.cppm
export module transform;

import component; // Inherits from Component
import <vector>;
import <memory>; // weak_ptr, shared_ptr
import <algorithm>; // std::find, std::remove
import <stdexcept>; // runtime_error

// Assuming these are available via include or modules
#include <engine/data/Vector3.hpp>
#include <engine/data/Quaternion.hpp>
#include <engine/data/Matrix4x4.hpp>
#include <cmath>

// Forward declare SceneObject again if needed by methods here
// class SceneObject; - Already done via 'component' import potentially

export class Transform : public Component
{
    private:
        // Local space data
        Vector3 m_localPosition;
        Quaternion m_localRotation;
        Vector3 m_localScale;

        // Hierarchy data
        std::weak_ptr<Transform> m_parent;
        std::vector<Transform*> m_children; // Raw pointers! SceneObject owner manages lifetime.

        // --- Internal Helper ---
        // Detaches this transform from its current parent's children list
        void detach_from_parent()
        {
            if (auto p = m_parent.lock())
            {
                auto& siblings = p->m_children;
                siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
            }
        }

        // Attaches this transform to a new parent's children list
        void attach_to_parent(Transform* newParentPtr)
        {
            if (newParentPtr)
            {
                // Check if already a child (shouldn't happen if detach_from_parent worked)
                if (std::find(newParentPtr->m_children.begin(), newParentPtr->m_children.end(), this) == newParentPtr->m_children.end())
                {
                    newParentPtr->m_children.push_back(this);
                }
            }
        }

        // Recalculates local transform based on current world and new parent
        void update_local_transform(const Matrix4x4& oldWorldMatrix, Transform* newParentPtr)
        {
            Matrix4x4 newParentWorldInv = newParentPtr ? newParentPtr->get_world_to_local_matrix() : Matrix4x4::identity();
            Matrix4x4 newLocalMatrix = newParentWorldInv * oldWorldMatrix;

            // Decompose newLocalMatrix into position, rotation, scale
            // This is non-trivial if shear is involved. Assuming standard TRS decomposition.
            m_localPosition = newLocalMatrix.get_translation();
            m_localRotation = Quaternion(newLocalMatrix).normalized(); // Extract rotation
            // Scale extraction is tricky, simplified version:
            Vector3 parentScale = newParentPtr ? newParentPtr->get_world_scale() : Vector3::one();
            // Avoid division by zero or tiny numbers
            float sx = std::abs(parentScale.x) > 1e-6f ? newLocalMatrix.get_x_axis().magnitude() : 0.0f;
            float sy = std::abs(parentScale.y) > 1e-6f ? newLocalMatrix.get_y_axis().magnitude() : 0.0f;
            float sz = std::abs(parentScale.z) > 1e-6f ? newLocalMatrix.get_z_axis().magnitude() : 0.0f;
            m_localScale = Vector3(sx, sy, sz);
            // A more robust decomposition is needed for complex scenarios (especially non-uniform parent scale)
        }


    public:
        Transform()
            : Component("Transform"),
              m_localPosition(Vector3::zero()),
              m_localRotation(Quaternion::identity()),
              m_localScale(Vector3::one())
        {}

        ~Transform() override
        {
            // When a Transform is destroyed (because its SceneObject is destroyed):
            // 1. Detach from parent
            detach_from_parent();

            // 2. Orphan children: Set their parent pointers to null (or a scene root if applicable)
            // Create a copy because set_parent modifies the original list indirectly.
            auto children_copy = m_children;

            for (Transform* child : children_copy)
            {
                if (child)
                {
                    child->set_parent(nullptr); // This will call detach_from_parent(this) on the child
                }
            }

            m_children.clear(); // Ensure our list is empty
        }

        // Prevent copying/moving to avoid issues with hierarchy pointers
        Transform(const Transform&) = delete;
        Transform& operator=(const Transform&) = delete;
        Transform(Transform&&) = delete;
        Transform& operator=(Transform&&) = delete;

        // --- Hierarchy ---

        // Sets the parent of the transform. Pass nullptr to make it a root object.
        // Preserves world position/rotation/scale.
        void set_parent(Transform* newParentPtr)
        {
            // 1. Check for parenting to self or descendant (cycle check)
            if (newParentPtr == this)
            {
                // std::cerr << "Warning: Cannot parent a Transform to itself." << std::endl;
                return;
            }

            Transform* ancestor = newParentPtr;

            while(ancestor)
            {
                if (ancestor == this)
                {
                    // std::cerr << "Warning: Cannot parent a Transform to one of its children." << std::endl;
                    return;
                }

                ancestor = ancestor->get_raw_parent_ptr(); // Need a way to get raw ptr for loop
            }

            // 2. Store current world transform
            Matrix4x4 worldMat = get_world_matrix(); // Calculate BEFORE changing parent

            // 3. Detach from the old parent
            detach_from_parent();

            // 4. Update parent weak_ptr
            if (newParentPtr)
            {
                // Try to get a shared_ptr to the new parent's SceneObject to create weak_ptr
                // This is awkward. A direct weak_ptr<Transform> requires the parent Transform
                // to be managed by shared_ptr, which it isn't directly.
                // Option A: Store raw pointer only (simpler, riskier if parent deleted unexpectedly)
                // Option B: Store weak_ptr<SceneObject> of parent, get transform from that (safer)
                // Let's try Option A for now (raw pointer) matching m_children style.
                // *** REVISED: Using weak_ptr IS better. Assume we can get one if needed ***
                // Let's assume the user passes a valid pointer obtained from a SceneObject.
                // We need the shared_ptr of the *component* itself if we want enable_shared_from_this...
                // Sticking to weak_ptr<Transform> implies Transform must support shared_from_this,
                // which it can via Component->Object base. BUT it must be created via shared_ptr.
                // This design is getting complex. Let's simplify and use raw pointers primarily,
                // relying on SceneObject lifecycle.

                // ** SIMPLIFIED APPROACH **
                // Use weak_ptr for safety if possible, otherwise manage raw ptr carefully.
                // Let's keep the weak_ptr member, but setting it might require getting the owning SO's shared_ptr.
                // For now, assume the `newParentPtr` lives as long as needed.
                m_parent = newParentPtr ? newParentPtr->shared_from_this() : std::weak_ptr<Transform>(); // Requires Transform supports shared_from_this
                // *** If shared_from_this on Transform is unreliable/not setup, use raw ptr only ***
                // m_parent_raw = newParentPtr; // Alternative if weak_ptr is problematic
            }
            else
            {
                m_parent.reset();
            }


            // 5. Attach to the new parent's children list
            attach_to_parent(newParentPtr);

            // 6. Recalculate local position/rotation/scale based on old world matrix
            update_local_transform(worldMat, newParentPtr);

            // 7. Invalidate caches if any (e.g., world matrix cache)
            // invalidate_world_matrix_recursive();
        }


        // Gets the parent transform, returns nullptr if this is a root transform.
        std::shared_ptr<Transform> get_parent() const
        {
            return m_parent.lock();
        }

        // Helper for cycle check
        Transform* get_raw_parent_ptr() const
        {
            // If using weak_ptr, locking is needed, which is slow for a loop.
            // If storing a raw ptr alternative: return m_parent_raw;
            // If only weak_ptr: Requires locking or alternative tracking.
            // This highlights complexity. Let's assume get_parent().get() is okay for now.
            auto p = m_parent.lock();
            return p.get();
        }


        int get_child_count() const
        {
            return static_cast<int>(m_children.size());
        }

        // Gets the child transform at the specified index. Returns nullptr if index is out of bounds.
        Transform* get_child(int index) const
        {
            if (index >= 0 && index < m_children.size())
            {
                return m_children[index];
            }

            return nullptr;
        }

        // Find child by name (requires SceneObject access)
        // Transform* find(const std::string& name); // Implementation below

        // --- Local Space ---
        const Vector3& get_local_position() const { return m_localPosition; }

        void set_local_position(const Vector3& pos) { m_localPosition = pos; }

        const Quaternion& get_local_rotation() const { return m_localRotation; }

        void set_local_rotation(const Quaternion& rot) { m_localRotation = rot.normalized(); }

        const Vector3& get_local_scale() const { return m_localScale; }

        void set_local_scale(const Vector3& scl) { m_localScale = scl; }

        Matrix4x4 get_local_matrix() const
        {
            return Matrix4x4::trs(m_localPosition, m_localRotation, m_localScale);
        }

        // --- World Space ---
        Matrix4x4 get_world_matrix() const
        {
            // Recursively calculates the world matrix
            Matrix4x4 localMat = get_local_matrix();

            if (auto p = m_parent.lock())
            {
                return p->get_world_matrix() * localMat;
            }
            else
            {
                return localMat; // This is a root transform
            }
        }

        Matrix4x4 get_world_to_local_matrix() const { return get_world_matrix().inverse(); }

        Vector3 get_world_position() const { return get_world_matrix().get_translation(); }

        Quaternion get_world_rotation() const { return Quaternion(get_world_matrix()).normalized(); }

        Vector3 get_world_scale() const   /* ... (same complex scale extraction as before) ... */
        {
            Matrix4x4 worldMat = get_world_matrix();
            float sx = worldMat.get_x_axis().magnitude();
            float sy = worldMat.get_y_axis().magnitude();
            float sz = worldMat.get_z_axis().magnitude();
            return Vector3(sx, sy, sz);
        }

        void set_world_position(const Vector3& worldPos)
        {
            Matrix4x4 parentWorldInv = Matrix4x4::identity();

            if (auto p = m_parent.lock())
            {
                parentWorldInv = p->get_world_to_local_matrix();
            }

            set_local_position(parentWorldInv.transform_point(worldPos));
        }

        void set_world_rotation(const Quaternion& worldRot)
        {
            Quaternion targetWorldRot = worldRot.normalized();
            Quaternion parentWorldRotInv = Quaternion::identity();

            if (auto p = m_parent.lock())
            {
                parentWorldRotInv = p->get_world_rotation().inverse();
            }

            set_local_rotation(parentWorldRotInv * targetWorldRot);
        }

        // Note: Setting world scale directly is problematic due to parent scale interactions.
        // Usually only local scale is set.

        // --- Directions ---
        Vector3 get_forward() const { return get_world_rotation() * Vector3::forward(); }

        Vector3 get_right() const { return get_world_rotation() * Vector3::right(); }

        Vector3 get_up() const { return get_world_rotation() * Vector3::up(); }

        // --- Operations ---
        void look_at(const Vector3& worldTarget, const Vector3& worldUp = Vector3::up())
        {
            Vector3 currentWorldPos = get_world_position();
            Vector3 direction = (worldTarget - currentWorldPos).normalized();

            if (direction.sqr_magnitude() > 1e-6f)
            {
                set_world_rotation(Quaternion::look_rotation(direction, worldUp));
            }
        }

        void translate(const Vector3& translation, bool localSpace = true)
        {
            if (localSpace)
            {
                m_localPosition += m_localRotation * translation;
            }
            else     // World space
            {
                set_world_position(get_world_position() + translation);
            }
        }

        void rotate(const Vector3& eulerAngles, bool localSpace = true)
        {
            rotate(Quaternion::euler(eulerAngles), localSpace);
        }

        void rotate(const Quaternion& rotation, bool localSpace = true)
        {
            Quaternion normRot = rotation.normalized();

            if (localSpace)
            {
                m_localRotation = (m_localRotation * normRot).normalized();
            }
            else     // World space
            {
                set_world_rotation(normRot * get_world_rotation());
            }
        }

        // --- Coordinate Transformation ---
        Vector3 transform_point(const Vector3& localPoint) const { return get_world_matrix().transform_point(localPoint); }

        Vector3 transform_direction(const Vector3& localDirection) const { return get_world_matrix().transform_direction(localDirection); }

        Vector3 inverse_transform_point(const Vector3& worldPoint) const { return get_world_to_local_matrix().transform_point(worldPoint); }

        Vector3 inverse_transform_direction(const Vector3& worldDirection) const { return get_world_to_local_matrix().transform_direction(worldDirection); }

        // --- Friend declaration needed for SceneObject potentially ---
        friend class SceneObject; // Allow SceneObject to manage Transform lifecycle if needed
};