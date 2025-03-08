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
            UpdateLocalToWorldMatrix();
        }

        Transform(const Vector3& pos, const Quaternion& rot = Quaternion::identity(), const Vector3& scl = Vector3(1.0f, 1.0f, 1.0f))
            : position(pos),
              rotation(rot),
              scale(scl),
              parent(nullptr),
              refCount(1)
        {
            UpdateLocalToWorldMatrix();
        }

        // Reference counting methods
        void AddRef()
        {
            refCount++;
        }

        void Release()
        {
            if(--refCount == 0)
                delete this;
        }

        // Space enum for relative transformations
        enum class Space { Self, World };

        // Position methods
        Vector3 GetPosition() const { return position; }

        void SetPosition(const Vector3& pos) { position = pos; UpdateLocalToWorldMatrix(); }

        void Translate(const Vector3& translation) { position += translation; UpdateLocalToWorldMatrix(); }

        void Translate(const Vector3& translation, Space relativeTo);

        // Rotation methods
        Quaternion GetRotation() const { return rotation; }

        void SetRotation(const Quaternion& rot) { rotation = rot; UpdateLocalToWorldMatrix(); }

        void Rotate(const Vector3& eulerAngles);
        void Rotate(const Vector3& axis, float angle);
        void Rotate(const Quaternion& rot) { rotation = rot * rotation; UpdateLocalToWorldMatrix(); }

        void LookAt(const Vector3& target, const Vector3& worldUp = Vector3(0.0f, 1.0f, 0.0f));

        // Scale methods
        Vector3 GetScale() const { return scale; }

        void SetScale(const Vector3& scl) { scale = scl; UpdateLocalToWorldMatrix(); }

        void SetScale(float uniformScale) { scale = Vector3(uniformScale, uniformScale, uniformScale); UpdateLocalToWorldMatrix(); }

        // Local axis getters
        Vector3 GetForward() const { return (rotation * Vector3(0.0f, 0.0f, -1.0f)).normalized(); }

        Vector3 GetRight() const { return (rotation * Vector3(1.0f, 0.0f, 0.0f)).normalized(); }

        Vector3 GetUp() const { return (rotation * Vector3(0.0f, 1.0f, 0.0f)).normalized(); }

        // Matrix operations
        Matrix4x4 GetLocalToWorldMatrix() const { return localToWorldMatrix; }

        Matrix4x4 GetWorldToLocalMatrix() const { return localToWorldMatrix.inverse(); }

        // Hierarchy operations
        Transform* GetParent() const { return parent; }

        void SetParent(Transform* newParent);
        Vector3 TransformPoint(const Vector3& point) const;
        Vector3 TransformDirection(const Vector3& direction) const;
        Vector3 InverseTransformPoint(const Vector3& worldPoint) const;
        Vector3 InverseTransformDirection(const Vector3& worldDirection) const;

    private:
        Vector3 position;
        Quaternion rotation;
        Vector3 scale;
        Transform* parent;
        Matrix4x4 localToWorldMatrix;
        int refCount; // Reference counter for memory management

        void UpdateLocalToWorldMatrix()
        {
            localToWorldMatrix = Matrix4x4::TRS(position, rotation, scale);

            if (parent)
            {
                localToWorldMatrix = parent->GetLocalToWorldMatrix() * localToWorldMatrix;
            }
        }
};

// Implementation of deferred methods
void Transform::Translate(const Vector3& translation, Space relativeTo)
{
    if (relativeTo == Space::Self)
    {
        position += rotation * translation;
    }
    else // Space::World
    {
        position += translation;
    }

    UpdateLocalToWorldMatrix();
}

void Transform::Rotate(const Vector3& eulerAngles)
{
    rotation = Quaternion::euler(eulerAngles) * rotation;
    UpdateLocalToWorldMatrix();
}

void Transform::Rotate(const Vector3& axis, float angle)
{
    rotation = Quaternion::angleAxis(angle, axis.normalized()) * rotation;
    UpdateLocalToWorldMatrix();
}

void Transform::LookAt(const Vector3& target, const Vector3& worldUp)
{
    Vector3 direction = (target - position).normalized();

    if (direction.sqrMagnitude() > 0.0f)
    {
        rotation = Quaternion::lookRotation(direction, worldUp);
        UpdateLocalToWorldMatrix();
    }
}

void Transform::SetParent(Transform* newParent)
{
    if (newParent == this)
        return;

    // Handle reference counting for parent change
    if (newParent)
        newParent->AddRef();

    if (parent)
        parent->Release();

    // Convert to world space
    Vector3 worldPos = position;
    Quaternion worldRot = rotation;
    Vector3 worldScale = scale;

    // Set new parent
    parent = newParent;

    if (parent)
    {
        // Convert to local space of new parent
        Matrix4x4 parentWorldToLocal = parent->GetWorldToLocalMatrix();
        position = parentWorldToLocal.transformPoint(worldPos);
        rotation = parent->GetRotation().inverse() * worldRot;

        // Scale calculation is simplified here, would need to account for non-uniform parent scales properly
        Vector3 parentScale = parent->GetScale();
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

    UpdateLocalToWorldMatrix();
}

Vector3 Transform::TransformPoint(const Vector3& point) const
{
    return localToWorldMatrix.transformPoint(point);
}

Vector3 Transform::TransformDirection(const Vector3& direction) const
{
    return localToWorldMatrix.transformDirection(direction);
}

Vector3 Transform::InverseTransformPoint(const Vector3& worldPoint) const
{
    return GetWorldToLocalMatrix().transformPoint(worldPoint);
}

Vector3 Transform::InverseTransformDirection(const Vector3& worldDirection) const
{
    return GetWorldToLocalMatrix().transformDirection(worldDirection);
}