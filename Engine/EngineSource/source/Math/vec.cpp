#include "Math/vec.h"
#include <cmath>
using namespace Engine;

// Method implementations
float vec3::length() const {
    return std::sqrtf(x * x + y * y + z * z);
}

float vec3::length_squared() const {
    return x * x + y * y + z * z;
}

vec3 vec3::normalized() const {
    float len = length();
    return vec3(x / len, y / len, z / len);
}

vec3& Engine::vec3::operator=(const vec4& v)
{
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}

vec3 vec3::operator+(const vec3& v) const {
    return vec3(x + v.x, y + v.y, z + v.z);
}

vec3& Engine::vec3::operator+=(const vec3& v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

vec3 vec3::operator-(const vec3& v) const {
    return vec3(x - v.x, y - v.y, z - v.z);
}

vec3& Engine::vec3::operator=(const vec3& v)
{
    x = v.x;
    y = v.y;
    z = v.z;

    return *this;
}

bool Engine::vec3::operator==(const vec3& v) const
{
    return (x == v.x) && (y == v.y) && (z == v.z);
}

bool Engine::vec3::operator!=(const vec3& v) const
{
    return !((x == v.x) && (y == v.y) && (z == v.z));
}

vec3 vec3::operator*(float t) const {
    return vec3(x * t, y * t, z * t);
}

vec3 vec3::operator/(float t) const {
    return vec3(x / t, y / t, z / t);
}

vec3 vec3::operator-() const {
    return vec3(-x, -y, -z);
}

Engine::vec4::vec4(const vec3& v, float w) : x(v.x), y(v.y), z(v.z), w(w)
{
}

float Engine::vec4::length() const
{
    return std::sqrtf(x * x + y * y + z * z + w * w);
}

float Engine::vec4::length_squared() const
{
    return (x * x + y * y + z * z);;
}

vec4 Engine::vec4::normalized() const
{
    float len = length();
    return vec4(x / len, y / len, z / len, w / len);
}

vec4 Engine::vec4::operator+(const vec4& v) const
{
    return vec4(x + v.x, y + v.y, z + v.z, w + v.w);
}

vec4 Engine::vec4::operator-(const vec4& v) const
{
    return vec4(x - v.x, y - v.y, z - v.z, w - v.w);
}

vec4 Engine::vec4::operator*(const vec4& v) const
{
    return vec4(x * v.x, y * v.y, z * v.z, w * v.w);
}

vec4 Engine::vec4::operator*(float t) const
{
    return vec4(x * t, y * t, z * t, w * t);
}

vec4 Engine::vec4::operator/(float t) const
{
    return vec4(x / t, y / t, z / t, w / t);
}

vec4 Engine::vec4::operator-() const
{
    return vec4(-x, -y, -z, -w);
}
