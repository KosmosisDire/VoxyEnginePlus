#pragma once
#include <engine/data/Vector3.hpp>
#include <engine/data/Quaternion.hpp>
#include <engine/data/Matrix4x4.hpp>

class Transform
{
    public:
        Transform()
            : position(Vector3(0.0f, 0.0f, 0.0f)),
              rotation(Quaternion::identity()),
              scale(Vector3(1.0f, 1.0f, 1.0f)),
              parent(nullptr),
              refCount(1)
        {
            update_local_to_world_matrix();
        }

        Transform(const Vector3& pos, const Quaternion& rot = Quaternion::identity(), const Vector3& scl = Vector3(1.0f, 1.0f, 1.0f))
            : position(pos),
              rotation(rot),
              scale(scl),
              parent(nullptr),
              refCount(1)
        {
            update_local_to_world_matrix();
        }

        // Reference counting methods
        void add_ref()
        {
            refCount++;
        }

        void release()
        {
            if(--refCount == 0)
                delete this;
        }

        // Space enum for relative transformations
        enum class Space { Self, World };

        // Position methods
        Vector3 get_position() const { return position; }

        void set_position(const Vector3& pos) { position = pos; update_local_to_world_matrix(); }

        void translate(const Vector3& translation) { position += translation; update_local_to_world_matrix(); }

        void translate(const Vector3& translation, Space relativeTo);

        // Rotation methods
        Quaternion get_rotation() const { return rotation; }

        void set_rotation(const Quaternion& rot) { rotation = rot; update_local_to_world_matrix(); }

        void rotate(const Vector3& eulerAngles);
        void rotate(const Vector3& axis, float angle);
        void rotate(const Quaternion& rot) { rotation = rot * rotation; update_local_to_world_matrix(); }

        void look_at(const Vector3& target, const Vector3& worldUp = Vector3(0.0f, 1.0f, 0.0f));

        // Scale methods
        Vector3 get_scale() const { return scale; }

        void set_scale(const Vector3& scl) { scale = scl; update_local_to_world_matrix(); }

        void set_scale(float uniformScale) { scale = Vector3(uniformScale, uniformScale, uniformScale); update_local_to_world_matrix(); }

        // Local axis getters
        Vector3 get_forward() const { return (rotation * Vector3(0.0f, 0.0f, -1.0f)).normalized(); }

        Vector3 get_right() const { return (rotation * Vector3(1.0f, 0.0f, 0.0f)).normalized(); }

        Vector3 get_up() const { return (rotation * Vector3(0.0f, 1.0f, 0.0f)).normalized(); }

        // Matrix operations
        Matrix4x4 get_local_to_world_matrix() const { return localToWorldMatrix; }

        Matrix4x4 get_world_to_local_matrix() const { return localToWorldMatrix.inverse(); }

        // Hierarchy operations
        Transform* get_parent() const { return parent; }

        void set_parent(Transform* newParent);
        Vector3 transform_point(const Vector3& point) const;
        Vector3 transform_direction(const Vector3& direction) const;
        Vector3 inverse_transform_point(const Vector3& worldPoint) const;
        Vector3 inverse_transform_direction(const Vector3& worldDirection) const;

    private:
        Vector3 position;
        Quaternion rotation;
        Vector3 scale;
        Transform* parent;
        Matrix4x4 localToWorldMatrix;
        int refCount; // Reference counter for memory management

        void update_local_to_world_matrix()
        {
            localToWorldMatrix = Matrix4x4::trs(position, rotation, scale);

            if (parent)
            {
                localToWorldMatrix = parent->get_local_to_world_matrix() * localToWorldMatrix; // Keep this call as is for now, assuming parent might be external or renamed later. Let's rename the declaration first.
            }
        }
};

// Implementation of deferred methods
void Transform::translate(const Vector3& translation, Space relativeTo)
{
    if (relativeTo == Space::Self)
    {
        position += rotation * translation;
    }
    else // Space::World
    {
        position += translation;
    }

    update_local_to_world_matrix();
}

void Transform::rotate(const Vector3& eulerAngles)
{
    rotation = Quaternion::euler(eulerAngles) * rotation;
    update_local_to_world_matrix();
}

void Transform::rotate(const Vector3& axis, float angle)
{
    rotation = Quaternion::angle_axis(angle, axis.normalized()) * rotation;
    update_local_to_world_matrix();
}

void Transform::look_at(const Vector3& target, const Vector3& worldUp)
{
    Vector3 direction = (target - position).normalized();

    if (direction.sqr_magnitude() > 0.0f)
    {
        rotation = Quaternion::look_rotation(direction, worldUp);
        update_local_to_world_matrix();
    }
}

void Transform::set_parent(Transform* newParent)
{
    if (newParent == this)
        return;

    // Handle reference counting for parent change
    if (newParent)
        newParent->add_ref(); // Fix call

    if (parent)
        parent->release(); // Fix call

    // Convert to world space
    Vector3 worldPos = position;
    Quaternion worldRot = rotation;
    Vector3 worldScale = scale;

    // Set new parent
    parent = newParent;

    if (parent)
    {
        // Convert to local space of new parent
        Matrix4x4 parentWorldToLocal = parent->get_world_to_local_matrix(); // Fix call
        position = parentWorldToLocal.transform_point(worldPos);
        rotation = parent->get_rotation().inverse() * worldRot; // Fix call

        // Scale calculation is simplified here, would need to account for non-uniform parent scales properly
        Vector3 parentScale = parent->get_scale(); // Fix call
        scale = Vector3(
                    worldScale.x / parentScale.x,
                    worldScale.y / parentScale.y,
                    worldScale.z / parentScale.z
                );
    }
    else
    {
        // If no parent, keep world values
        position = worldPos;
        rotation = worldRot;
        scale = worldScale;
    }

    update_local_to_world_matrix(); // Fix call
}

Vector3 Transform::transform_point(const Vector3& point) const
{
    return localToWorldMatrix.transform_point(point);
}

Vector3 Transform::transform_direction(const Vector3& direction) const
{
    return localToWorldMatrix.transform_direction(direction);
}

Vector3 Transform::inverse_transform_point(const Vector3& worldPoint) const
{
    return get_world_to_local_matrix().transform_point(worldPoint); // Fix call
}

Vector3 Transform::inverse_transform_direction(const Vector3& worldDirection) const
{
    return get_world_to_local_matrix().transform_direction(worldDirection); // Fix call
}
