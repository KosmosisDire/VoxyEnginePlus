#pragma once

#include <daxa/daxa.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// convert between glm and daxa types
inline auto to_daxa(glm::vec2 const &v) -> daxa_f32vec2
{
    return daxa_f32vec2{v.x, v.y};
}

inline auto to_daxa(glm::vec3 const &v) -> daxa_f32vec3
{
    return daxa_f32vec3{v.x, v.y, v.z};
}

inline auto to_daxa(glm::vec4 const &v) -> daxa_f32vec4
{
    return daxa_f32vec4{v.x, v.y, v.z, v.w};
}

auto to_daxa(glm::mat4 const &mat) -> daxa_f32mat4x4
{
    return daxa_f32mat4x4{
        {mat[0][0], mat[0][1], mat[0][2], mat[0][3]},
        {mat[1][0], mat[1][1], mat[1][2], mat[1][3]},
        {mat[2][0], mat[2][1], mat[2][2], mat[2][3]},
        {mat[3][0], mat[3][1], mat[3][2], mat[3][3]},
    };
}

auto to_glm(daxa_f32vec2 const &v) -> glm::vec2
{
    return glm::vec2{v.x, v.y};
}

auto to_glm(daxa_f32vec3 const &v) -> glm::vec3
{
    return glm::vec3{v.x, v.y, v.z};
}

auto to_glm(daxa_f32vec4 const &v) -> glm::vec4
{
    return glm::vec4{v.x, v.y, v.z, v.w};
}

auto to_glm(daxa_f32mat4x4 const &mat) -> glm::mat4
{
    return glm::mat4{
        {mat.x.x, mat.x.y, mat.x.z, mat.x.w},
        {mat.y.x, mat.y.y, mat.y.z, mat.y.w},
        {mat.z.x, mat.z.y, mat.z.z, mat.z.w},
        {mat.w.x, mat.w.y, mat.w.z, mat.w.w},
    };
}

auto daxa_mat_from_axis_angle(daxa_f32vec3 const &axis, float angle) -> daxa_f32mat4x4
{
    return to_daxa(glm::rotate(glm::mat4{1.0f}, angle, to_glm(axis)));
}

auto daxa_transform_normal(daxa_f32vec3 const &normal, daxa_f32mat4x4 const &mat) -> daxa_f32vec3
{
    return to_daxa(glm::normalize(glm::mat3{to_glm(mat)} * to_glm(normal)));
}

auto daxa_transform(daxa_f32vec3 const &v, daxa_f32mat4x4 const &mat) -> daxa_f32vec3
{
    return to_daxa(glm::vec3{to_glm(mat) * glm::vec4{to_glm(v), 1.0f}});
}

// add math operators to daxa types

// vec2 operators
inline auto operator+(daxa_f32vec2 const &lhs, daxa_f32vec2 const &rhs) -> daxa_f32vec2
{
    return to_daxa(to_glm(lhs) + to_glm(rhs));
}

inline auto operator-(daxa_f32vec2 const &lhs, daxa_f32vec2 const &rhs) -> daxa_f32vec2
{
    return to_daxa(to_glm(lhs) - to_glm(rhs));
}

inline auto operator*(daxa_f32vec2 const &lhs, daxa_f32vec2 const &rhs) -> daxa_f32vec2
{
    return to_daxa(to_glm(lhs) * to_glm(rhs));
}

inline auto operator/(daxa_f32vec2 const &lhs, daxa_f32vec2 const &rhs) -> daxa_f32vec2
{
    return to_daxa(to_glm(lhs) / to_glm(rhs));
}

inline auto operator*(daxa_f32vec2 const &v, float s) -> daxa_f32vec2
{
    return to_daxa(to_glm(v) * s);
}

inline auto operator*(float s, daxa_f32vec2 const &v) -> daxa_f32vec2
{
    return to_daxa(s * to_glm(v));
}

inline auto operator/(daxa_f32vec2 const &v, float s) -> daxa_f32vec2
{
    return to_daxa(to_glm(v) / s);
}

inline auto operator-(daxa_f32vec2 const &v) -> daxa_f32vec2
{
    return to_daxa(-to_glm(v));
}

// vec3 operators
inline auto operator+(daxa_f32vec3 const &lhs, daxa_f32vec3 const &rhs) -> daxa_f32vec3
{
    return to_daxa(to_glm(lhs) + to_glm(rhs));
}

inline auto operator-(daxa_f32vec3 const &lhs, daxa_f32vec3 const &rhs) -> daxa_f32vec3
{
    return to_daxa(to_glm(lhs) - to_glm(rhs));
}

inline auto operator*(daxa_f32vec3 const &lhs, daxa_f32vec3 const &rhs) -> daxa_f32vec3
{
    return to_daxa(to_glm(lhs) * to_glm(rhs));
}

inline auto operator/(daxa_f32vec3 const &lhs, daxa_f32vec3 const &rhs) -> daxa_f32vec3
{
    return to_daxa(to_glm(lhs) / to_glm(rhs));
}

inline auto operator*(daxa_f32vec3 const &v, float s) -> daxa_f32vec3
{
    return to_daxa(to_glm(v) * s);
}

inline auto operator*(float s, daxa_f32vec3 const &v) -> daxa_f32vec3
{
    return to_daxa(s * to_glm(v));
}

inline auto operator/(daxa_f32vec3 const &v, float s) -> daxa_f32vec3
{
    return to_daxa(to_glm(v) / s);
}

inline auto operator-(daxa_f32vec3 const &v) -> daxa_f32vec3
{
    return to_daxa(-to_glm(v));
}

// vec4 operators
inline auto operator+(daxa_f32vec4 const &lhs, daxa_f32vec4 const &rhs) -> daxa_f32vec4
{
    return to_daxa(to_glm(lhs) + to_glm(rhs));
}

inline auto operator-(daxa_f32vec4 const &lhs, daxa_f32vec4 const &rhs) -> daxa_f32vec4
{
    return to_daxa(to_glm(lhs) - to_glm(rhs));
}

inline auto operator*(daxa_f32vec4 const &lhs, daxa_f32vec4 const &rhs) -> daxa_f32vec4
{
    return to_daxa(to_glm(lhs) * to_glm(rhs));
}

inline auto operator/(daxa_f32vec4 const &lhs, daxa_f32vec4 const &rhs) -> daxa_f32vec4
{
    return to_daxa(to_glm(lhs) / to_glm(rhs));
}

inline auto operator*(daxa_f32vec4 const &v, float s) -> daxa_f32vec4
{
    return to_daxa(to_glm(v) * s);
}

inline auto operator*(float s, daxa_f32vec4 const &v) -> daxa_f32vec4
{
    return to_daxa(s * to_glm(v));
}

inline auto operator/(daxa_f32vec4 const &v, float s) -> daxa_f32vec4
{
    return to_daxa(to_glm(v) / s);
}

inline auto operator-(daxa_f32vec4 const &v) -> daxa_f32vec4
{
    return to_daxa(-to_glm(v));
}

// Matrix multiplication
inline auto operator*(daxa_f32mat4x4 const &lhs, daxa_f32mat4x4 const &rhs) -> daxa_f32mat4x4
{
    return to_daxa(to_glm(lhs) * to_glm(rhs));
}

inline auto operator*(daxa_f32mat4x4 const &m, daxa_f32vec4 const &v) -> daxa_f32vec4
{
    return to_daxa(to_glm(m) * to_glm(v));
}