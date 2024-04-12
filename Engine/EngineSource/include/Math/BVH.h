#pragma once
#include <stdint.h>

namespace Engine
{
    class Mesh;
    struct vec3;
    struct ray;
    struct hitInfo;

    class BVH
    {
        static const uint8_t kNumPlaneSetNormals = 7;
        static const vec3 planeSetNormals[kNumPlaneSetNormals];

        struct Extents
        {
            Extents()
            {
                for (uint8_t i = 0; i < kNumPlaneSetNormals; ++i)
                    d[i][0] = INT16_MAX, d[i][1] = -INT16_MAX;
            }
            bool intersect(const ray& r, float* precomputedNumerator, float* precomputeDenominator, float& tNear, float& tFar, uint8_t& planeIndex) const;
            float d[kNumPlaneSetNormals][2];
        };

        Extents extents;
    public:
        BVH();
        BVH(const Mesh* mesh);
        const bool intersect(const ray& ray, hitInfo& isectData) const;
        ~BVH();

    };
}
