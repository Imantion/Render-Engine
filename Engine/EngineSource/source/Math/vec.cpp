#include "Math/vec.h"
#include <cmath>
using namespace Engine;

// Method implementations
float vec3::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

float vec3::length_squared() const {
    return x * x + y * y + z * z;
}

vec3 vec3::normalized() const {
    float len = length();
    return vec3(x / len, y / len, z / len);
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

vec3 vec3::operator*(float t) const {
    return vec3(x * t, y * t, z * t);
}

vec3 vec3::operator/(float t) const {
    return vec3(x / t, y / t, z / t);
}

vec3 vec3::operator-() const {
    return vec3(-x, -y, -z);
}
