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

vec3 Engine::vec3::operator*(const vec3& v) const
{
    return  vec3(x * v.x, y * v.y, z * v.z);
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

vec3& Engine::vec3::operator*=(float t)
{
    x *= t;
    y *= t;
    z *= t;

    return *this;
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

//
//#pragma once
//#include <stdint.h>
//
//
//namespace Engine
//{
//    class Mesh;
//    struct vec3;
//    struct ray;
//    struct hitInfo;
//
//    class BVH
//    {
//        static const uint8_t kNumPlaneSetNormals = 7;
//        static const vec3 planeSetNormals[kNumPlaneSetNormals];
//
//        struct Extents
//        {
//            Extents()
//            {
//                for (uint8_t i = 0; i < kNumPlaneSetNormals; ++i)
//                    d[i][0] = INT16_MAX, d[i][1] = -INT16_MAX;
//            }
//            bool intersect(const ray& r, float* precomputedNumerator, float* precomputeDenominator, float& tNear, float& tFar, uint8_t& planeIndex) const;
//            float d[kNumPlaneSetNormals][2];
//        };
//
//        Extents extents;
//    public:
//        BVH();
//        BVH(const Mesh* mesh);
//        const bool intersect(const ray& ray, hitInfo& isectData) const;
//        ~BVH();
//
//    };
//}

//#include "Math/BVH.h"
//#include "Math/vec.h"
//#include "Math/Mesh.h"
//#include "Math/math.h"
//
//const Engine::vec3 Engine::BVH::planeSetNormals[Engine::BVH::kNumPlaneSetNormals] = {
//    Engine::vec3(1, 0, 0),
//    Engine::vec3(0, 1, 0),
//    Engine::vec3(0, 0, 1),
//    Engine::vec3(sqrtf(3) / 3.f,  sqrtf(3) / 3.f, sqrtf(3) / 3.f),
//    Engine::vec3(-sqrtf(3) / 3.f,  sqrtf(3) / 3.f, sqrtf(3) / 3.f),
//    Engine::vec3(-sqrtf(3) / 3.f, -sqrtf(3) / 3.f, sqrtf(3) / 3.f),
//    Engine::vec3(sqrtf(3) / 3.f, -sqrtf(3) / 3.f, sqrtf(3) / 3.f) };
//
//Engine::BVH::BVH()
//{
//}
//
//Engine::BVH::BVH(const Mesh* mesh) : extents()
//{
//    for (size_t j = 0; j < kNumPlaneSetNormals; j++)
//    {
//        mesh->computeBounds(planeSetNormals[j], extents.d[j][0], extents.d[j][1]);
//    }
//}
//
//const bool Engine::BVH::intersect(const ray& ray, hitInfo& isectData) const
//{
//    float tClosest = FLT_MAX;
//    float precomputedNumerator[BVH::kNumPlaneSetNormals], precomputeDenominator[BVH::kNumPlaneSetNormals];
//    for (uint8_t i = 0; i < kNumPlaneSetNormals; ++i) {
//        precomputedNumerator[i] = dot(planeSetNormals[i], ray.origin);
//        precomputeDenominator[i] = dot(planeSetNormals[i], ray.direction);
//    }
//
//    float tNear = -FLT_MAX, tFar = FLT_MAX;
//    uint8_t planeIndex;
//
//    if (extents.intersect(ray, precomputedNumerator, precomputeDenominator, tNear, tFar, planeIndex)) {
//        isectData.normal = planeSetNormals[planeIndex];
//        isectData.t = tNear;
//        return true;
//    }
//    return false;
//}
//
//Engine::BVH::~BVH()
//{
//}
//
//bool Engine::BVH::Extents::intersect(const ray& r, float* precomputedNumerator, float* precomputeDenominator, float& tNear, float& tFar, uint8_t& planeIndex) const
//{
//    for (size_t i = 0; i < kNumPlaneSetNormals; i++)
//    {
//        float tn = (d[i][0] - precomputedNumerator[i]) / precomputeDenominator[i];
//        float tf = (d[i][1] - precomputedNumerator[i]) / precomputeDenominator[i];
//
//        if (precomputeDenominator[i] < 0) std::swap(tn, tf);
//        if (tn > tNear) tNear = tn, planeIndex = i;
//        if (tf < tFar) tFar = tf;
//        if (tNear > tFar) return false;
//    }
//    return true;
//}
