#pragma once

#include "quaternion.hpp"
#include "vector3.hpp"
#include "vector4.hpp"
#include <array>
#include <daxa/daxa.hpp>
#include <string>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan compatibility
#define GLM_FORCE_RIGHT_HANDED
#include "vector3.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


struct Matrix4x4
{
        // Store in column-major order for Vulkan compatibility
        std::array<float, 16> data;

        // Constructors
        Matrix4x4()
        {
            *this = identity();
        }

        Matrix4x4(const std::array<float, 16> &values) : data(values) {}

        // Direct element access (row, column) - converts from row-major input to column-major storage
        float& operator()(int row, int col)
        {
            return data[col * 4 + row];
        }

        const float& operator()(int row, int col) const
        {
            return data[col * 4 + row];
        }

        // Static factory methods
        static Matrix4x4 identity()
        {
            return Matrix4x4({1.0f, 0.0f, 0.0f, 0.0f,
                              0.0f, 1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 1.0f, 0.0f,
                              0.0f, 0.0f, 0.0f, 1.0f});
        }

        static Matrix4x4 translation(const Vector3 &translation)
        {
            glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));
            return from_glm(m); // Update call
        }

        static Matrix4x4 rotation(const Quaternion &rotation)
        {
            glm::mat4 m = glm::mat4_cast(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
            return from_glm(m); // Update call
        }

        static Matrix4x4 scale(const Vector3 &scale)
        {
            glm::mat4 m = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));
            return from_glm(m); // Update call
        }

        static Matrix4x4 trs(const Vector3 &translation, const Quaternion &rotation, const Vector3 &scale)
        {
            glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));
            glm::mat4 r = glm::mat4_cast(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
            glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));
            return from_glm(t * r * s); // Update call
        }

        static Matrix4x4 perspective(float fovY, float aspect, float nearPlane, float farPlane)
        {
            glm::mat4 m = glm::perspective(glm::radians(fovY), aspect, nearPlane, farPlane);
            return from_glm(m); // Update call
        }

        static Matrix4x4 orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
        {
            glm::mat4 m = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
            return from_glm(m); // Update call
        }

        static Matrix4x4 look_at(const Vector3 &position, const Vector3 &target, const Vector3 &up)
        {
            glm::mat4 m = glm::lookAt(
                              glm::vec3(position.x, position.y, position.z),
                              glm::vec3(target.x, target.y, target.z),
                              glm::vec3(up.x, up.y, up.z));
            return from_glm(m); // Update call
        }

        // Basic operators
        Matrix4x4 operator*(const Matrix4x4 &rhs) const
        {
            glm::mat4 a = to_glm(*this); // Update call
            glm::mat4 b = to_glm(rhs); // Update call
            return from_glm(a * b); // Update call
        }

        Vector4 operator*(const Vector4 &v) const
        {
            glm::mat4 m = to_glm(*this); // Update call
            glm::vec4 gv(v.x, v.y, v.z, v.w);
            glm::vec4 result = m * gv;
            return Vector4(result.x, result.y, result.z, result.w);
        }

        bool operator==(const Matrix4x4 &rhs) const
        {
            for (int i = 0; i < 16; ++i)
            {
                if (data[i] != rhs.data[i])
                {
                    return false;
                }
            }

            return true;
        }

        bool operator!=(const Matrix4x4 &rhs) const
        {
            return !(*this == rhs);
        }

        Vector3 multiply_point(const Vector3 &v) const
        {

        }

        // Matrix operations
        Matrix4x4 inverse() const
        {
            glm::mat4 m = to_glm(*this); // Update call
            return from_glm(glm::inverse(m)); // Update call
        }

        Matrix4x4 transpose() const
        {
            glm::mat4 m = to_glm(*this); // Update call
            return from_glm(glm::transpose(m)); // Update call
        }

        float determinant() const
        {
            glm::mat4 m = to_glm(*this); // Update call
            return glm::determinant(m);
        }

        void decompose(Vector3 &translation, Quaternion &rotation, Vector3 &scale) const
        {
            glm::mat4 m = to_glm(*this); // Update call
            glm::vec3 t, s;
            glm::quat r;

            t = m[3];

            for (int i = 0; i < 3; i++)
            {
                s[i] = glm::length(glm::vec3(m[i]));
            }

            const glm::mat3 rotMtx(
                glm::vec3(m[0]) / s[0],
                glm::vec3(m[1]) / s[1],
                glm::vec3(m[2]) / s[2]);
            r = glm::quat_cast(rotMtx);

            translation = Vector3(t.x, t.y, t.z);
            rotation = Quaternion(r.w, r.x, r.y, r.z);
            scale = Vector3(s.x, s.y, s.z);
        }

        // Access to raw data for graphics APIs
        const float* get_data() const
        {
            return data.data();
        }

        // Helper methods for GLM conversion
    private:
        static Matrix4x4 from_glm(const glm::mat4 &m)
        {
            return Matrix4x4({m[0][0], m[0][1], m[0][2], m[0][3],
                              m[1][0], m[1][1], m[1][2], m[1][3],
                              m[2][0], m[2][1], m[2][2], m[2][3],
                              m[3][0], m[3][1], m[3][2], m[3][3]});
        }

        static glm::mat4 to_glm(const Matrix4x4 &m)
        {
            return glm::make_mat4(m.data.data());
        }

    public:
        // String conversion
        std::string to_string() const
        {
            std::string result;

            for (int row = 0; row < 4; ++row)
            {
                result += row == 0 ? "[[" : " [";

                for (int col = 0; col < 4; ++col)
                {
                    result += std::to_string((*this)(row, col));

                    if (col < 3)
                    {
                        result += ", ";
                    }
                }

                result += row == 3 ? "]]" : "]\n";
            }

            return result;
        }

        // Daxa conversions
        [[nodiscard]] daxa_f32mat4x4 to_daxa() const
        {
            return daxa_f32mat4x4
            {
                {data[0], data[1], data[2], data[3]},
                {data[4], data[5], data[6], data[7]},
                {data[8], data[9], data[10], data[11]},
                {data[12], data[13], data[14], data[15]}};
        }

        static Matrix4x4 from_daxa(const daxa_f32mat4x4 &mat)
        {
            return Matrix4x4({mat.x.x, mat.x.y, mat.x.z, mat.x.w,
                              mat.y.x, mat.y.y, mat.y.z, mat.y.w,
                              mat.z.x, mat.z.y, mat.z.z, mat.z.w,
                              mat.w.x, mat.w.y, mat.w.z, mat.w.w});
        }

        // Utility functions using our custom types
        static Matrix4x4 from_axis_angle(const Vector3 &axis, float angle)
        {
            glm::mat4 m = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(axis.x, axis.y, axis.z));
            return from_glm(m); // Update call
        }

        // Transform a point with perspective division (handles projective transformations)
        Vector3 transform_point(const Vector3 &point) const
        {
            glm::mat4 m = to_glm(*this); // Update call
            glm::vec4 result = m * glm::vec4(point.x, point.y, point.z, 1.0f);

            // Apply perspective division
            if (result.w != 0.0f && result.w != 1.0f)
            {
                return Vector3(result.x / result.w, result.y / result.w, result.z / result.w);
            }

            return Vector3(result.x, result.y, result.z);
        }

        // Transform a point without perspective division (for affine transformations)
        Vector3 transform_point_affine(const Vector3 &point) const
        {
            glm::mat4 m = to_glm(*this); // Update call
            glm::vec4 result = m * glm::vec4(point.x, point.y, point.z, 1.0f);
            return Vector3(result.x, result.y, result.z);
        }

        // Transform a direction vector (ignores translation, preserves direction)
        Vector3 transform_direction(const Vector3 &direction) const
        {
            glm::mat4 m = to_glm(*this); // Update call
            glm::vec4 result = m * glm::vec4(direction.x, direction.y, direction.z, 0.0f);
            return Vector3(result.x, result.y, result.z);
        }

        // Transform a normal vector (uses inverse transpose to maintain perpendicularity)
        Vector3 transform_normal(const Vector3 &normal) const
        {
            // Use the inverse transpose of the upper 3x3 portion of the matrix
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(to_glm(*this)))); // Update call
            glm::vec3 result = normalMatrix * glm::vec3(normal.x, normal.y, normal.z);
            return Vector3(result.x, result.y, result.z);
        }
};
