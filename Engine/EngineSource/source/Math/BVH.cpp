#include "Math/BVH.h"
#include "Math/vec.h"
#include "Math/Mesh.h"
#include "Math/math.h"

const Engine::vec3 Engine::BVH::planeSetNormals[Engine::BVH::kNumPlaneSetNormals] = {
    Engine::vec3(1, 0, 0),
    Engine::vec3(0, 1, 0),
    Engine::vec3(0, 0, 1),
    Engine::vec3(sqrtf(3) / 3.f,  sqrtf(3) / 3.f, sqrtf(3) / 3.f),
    Engine::vec3(-sqrtf(3) / 3.f,  sqrtf(3) / 3.f, sqrtf(3) / 3.f),
    Engine::vec3(-sqrtf(3) / 3.f, -sqrtf(3) / 3.f, sqrtf(3) / 3.f),
    Engine::vec3(sqrtf(3) / 3.f, -sqrtf(3) / 3.f, sqrtf(3) / 3.f) };

Engine::BVH::BVH()
{
}

Engine::BVH::BVH(const Mesh* mesh) : extents()
{
    for (size_t j = 0; j < kNumPlaneSetNormals; j++)
    {
        mesh->computeBounds(planeSetNormals[j], extents.d[j][0], extents.d[j][1]);
    }
}

const bool Engine::BVH::intersect(const ray& ray, hitInfo& isectData) const
{
    float tClosest = FLT_MAX;
    float precomputedNumerator[BVH::kNumPlaneSetNormals], precomputeDenominator[BVH::kNumPlaneSetNormals];
    for (uint8_t i = 0; i < kNumPlaneSetNormals; ++i) {
        precomputedNumerator[i] = dot(planeSetNormals[i], ray.origin);
        precomputeDenominator[i] = dot(planeSetNormals[i], ray.direction);
    }

    float tNear = -FLT_MAX, tFar = FLT_MAX;
    uint8_t planeIndex;

    if (extents.intersect(ray, precomputedNumerator, precomputeDenominator, tNear, tFar, planeIndex)) {
        isectData.normal = planeSetNormals[planeIndex];
        isectData.t = tNear;
        return true;
    }
    return false;
}

Engine::BVH::~BVH()
{
}

bool Engine::BVH::Extents::intersect(const ray& r, float* precomputedNumerator, float* precomputeDenominator, float& tNear, float& tFar, uint8_t& planeIndex) const
{
    for (size_t i = 0; i < kNumPlaneSetNormals; i++)
    {
        float tn = (d[i][0] - precomputedNumerator[i]) / precomputeDenominator[i];
        float tf = (d[i][1] - precomputedNumerator[i]) / precomputeDenominator[i];

        if (precomputeDenominator[i] < 0) std::swap(tn, tf);
        if (tn > tNear) tNear = tn, planeIndex = i;
        if (tf < tFar) tFar = tf;
        if (tNear > tFar) return false;
    }
    return true;
}
