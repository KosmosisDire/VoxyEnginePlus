#pragma once

#include <cmath>
#include <string>
#include <array>
#include <daxa/daxa.hpp>
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "Quaternion.hpp"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE  // Vulkan compatibility
#define GLM_FORCE_RIGHT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "vector3.hpp"

struct Matrix4x4 {
    // Store in column-major order for Vulkan compatibility
    std::array<float, 16> data;

    // Constructors
    Matrix4x4() {
        *this = identity();
    }

    Matrix4x4(const std::array<float, 16>& values) : data(values) {}

    // Direct element access (row, column) - converts from row-major input to column-major storage
    float& operator()(int row, int col) {
        return data[col * 4 + row];
    }

    const float& operator()(int row, int col) const {
        return data[col * 4 + row];
    }

    // Static factory methods
    static Matrix4x4 identity() {
        return Matrix4x4({
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        });
    }

    static Matrix4x4 translate(const Vector3& translation) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));
        return fromGLM(m);
    }

    static Matrix4x4 rotate(const Quaternion& rotation) {
        glm::mat4 m = glm::mat4_cast(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
        return fromGLM(m);
    }

    static Matrix4x4 scale(const Vector3& scale) {
        glm::mat4 m = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));
        return fromGLM(m);
    }

    static Matrix4x4 trs(const Vector3& translation, const Quaternion& rotation, const Vector3& scale) {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));
        glm::mat4 r = glm::mat4_cast(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
        glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));
        return fromGLM(t * r * s);
    }

    static Matrix4x4 perspective(float fovY, float aspect, float nearPlane, float farPlane) {
        glm::mat4 m = glm::perspective(glm::radians(fovY), aspect, nearPlane, farPlane);
        return fromGLM(m);
    }

    static Matrix4x4 orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
        glm::mat4 m = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
        return fromGLM(m);
    }

    static Matrix4x4 lookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
        glm::mat4 m = glm::lookAt(
            glm::vec3(eye.x, eye.y, eye.z),
            glm::vec3(target.x, target.y, target.z),
            glm::vec3(up.x, up.y, up.z)
        );
        return fromGLM(m);
    }

    // Basic operators
    Matrix4x4 operator*(const Matrix4x4& rhs) const {
        glm::mat4 a = toGLM(*this);
        glm::mat4 b = toGLM(rhs);
        return fromGLM(a * b);
    }

    Vector4 operator*(const Vector4& v) const {
        glm::mat4 m = toGLM(*this);
        glm::vec4 gv(v.x, v.y, v.z, v.w);
        glm::vec4 result = m * gv;
        return Vector4(result.x, result.y, result.z, result.w);
    }

    Vector3 multiplyPoint(const Vector3& v) const {
        glm::mat4 m = toGLM(*this);
        glm::vec4 gv(v.x, v.y, v.z, 1.0f);
        glm::vec4 result = m * gv;
        return Vector3(result.x / result.w, result.y / result.w, result.z / result.w);
    }

    Vector3 multiplyVector(const Vector3& v) const {
        glm::mat4 m = toGLM(*this);
        glm::vec4 gv(v.x, v.y, v.z, 0.0f);
        glm::vec4 result = m * gv;
        return Vector3(result.x, result.y, result.z);
    }

    // Matrix operations
    Matrix4x4 inverse() const {
        glm::mat4 m = toGLM(*this);
        return fromGLM(glm::inverse(m));
    }

    Matrix4x4 transpose() const {
        glm::mat4 m = toGLM(*this);
        return fromGLM(glm::transpose(m));
    }

    float determinant() const {
        glm::mat4 m = toGLM(*this);
        return glm::determinant(m);
    }

    void decompose(Vector3& translation, Quaternion& rotation, Vector3& scale) const
    {
        glm::mat4 m = toGLM(*this);
        glm::vec3 t, s;
        glm::quat r;

        t = m[3];
        for(int i = 0; i < 3; i++)
            s[i] = glm::length(glm::vec3(m[i]));
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
    const float* getData() const {
        return data.data();
    }

    // Helper methods for GLM conversion
    private:
        static Matrix4x4 fromGLM(const glm::mat4& m) {
            return Matrix4x4({
                m[0][0], m[0][1], m[0][2], m[0][3],
                m[1][0], m[1][1], m[1][2], m[1][3],
                m[2][0], m[2][1], m[2][2], m[2][3],
                m[3][0], m[3][1], m[3][2], m[3][3]
            });
        }

        static glm::mat4 toGLM(const Matrix4x4& m) {
            return glm::make_mat4(m.data.data());
        }

    public:
        // String conversion
        std::string toString() const {
            std::string result;
            for (int row = 0; row < 4; ++row) {
                result += row == 0 ? "[[" : " [";
                for (int col = 0; col < 4; ++col) {
                    result += std::to_string((*this)(row, col));
                    if (col < 3) result += ", ";
                }
                result += row == 3 ? "]]" : "]\n";
            }
            return result;
        }

        // Daxa conversions
        [[nodiscard]] daxa_f32mat4x4 toDaxa() const {
            return daxa_f32mat4x4{
                {data[0], data[1], data[2], data[3]},
                {data[4], data[5], data[6], data[7]},
                {data[8], data[9], data[10], data[11]},
                {data[12], data[13], data[14], data[15]}
            };
        }

        static Matrix4x4 fromDaxa(const daxa_f32mat4x4& mat) {
            return Matrix4x4({
                mat.x.x, mat.x.y, mat.x.z, mat.x.w,
                mat.y.x, mat.y.y, mat.y.z, mat.y.w,
                mat.z.x, mat.z.y, mat.z.z, mat.z.w,
                mat.w.x, mat.w.y, mat.w.z, mat.w.w
            });
        }

        // Utility functions using our custom types
        static Matrix4x4 fromAxisAngle(const Vector3& axis, float angle) {
            glm::mat4 m = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(axis.x, axis.y, axis.z));
            return fromGLM(m);
        }

        Vector3 transformNormal(const Vector3& normal) const {
            glm::vec3 n = glm::normalize(glm::mat3(toGLM(*this)) * glm::vec3(normal.x, normal.y, normal.z));
            return Vector3(n.x, n.y, n.z);
        }

        Vector3 transformPoint(const Vector3& point) const {
            glm::vec4 p = toGLM(*this) * glm::vec4(point.x, point.y, point.z, 1.0f);
            return Vector3(p.x, p.y, p.z);
        }
};