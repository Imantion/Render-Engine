#include "Math/BVH.h"
#include "Math/vec.h"
#include "Math/Mesh.h"
#include <limits>
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

Engine::BVH::BVH(const mesh* mesh) : extents()
{
    computeBounds(mesh);
}

Engine::BVH::BVH(const BVH& bvh): extents(bvh.extents)
{
}

const bool Engine::BVH::intersect(const ray& ray, hitInfo& isectData) const
{
    float tClosest = std::numeric_limits<float>::max();
    float precomputedNumerator[BVH::kNumPlaneSetNormals], precomputeDenominator[BVH::kNumPlaneSetNormals];
    for (uint8_t i = 0; i < kNumPlaneSetNormals; ++i) {
        precomputedNumerator[i] = dot(planeSetNormals[i], ray.origin);
        precomputeDenominator[i] = dot(planeSetNormals[i], ray.direction);
    }

    float tNear = -std::numeric_limits<float>::max(), tFar = std::numeric_limits<float>::max();
    uint8_t planeIndex;

    if (extents.intersect(ray, precomputedNumerator, precomputeDenominator, tNear, tFar, planeIndex)) {
        isectData.normal = planeSetNormals[planeIndex];
        isectData.t = tNear;
        return true;
    }

    isectData.t = std::numeric_limits<float>::max();;
    return false;
}

void Engine::BVH::computeBounds(const mesh* mesh)
{
    for (size_t j = 0; j < kNumPlaneSetNormals; j++)
    {
        mesh->computeBounds(planeSetNormals[j], extents.d[j][0], extents.d[j][1]);
    }
}

Engine::BVH::~BVH()
{
}

bool Engine::BVH::Extents::intersect(const ray& r, float* precomputedNumerator, float* precomputeDenominator, float& tNear, float& tFar, uint8_t& planeIndex) const
{
    for (uint8_t i = 0; i < kNumPlaneSetNormals; i++)
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
