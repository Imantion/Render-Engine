#pragma once
#include <stdint.h>
#include <limits>
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
                    d[i][0] = std::numeric_limits<float>::max(), d[i][1] = -std::numeric_limits<float>::max();
            }
            Extents(const Extents& other)
            {
                for (size_t i = 0; i < kNumPlaneSetNormals; i++)
                {
                    d[i][0] = other.d[i][0];
                    d[i][1] = other.d[i][1];
                }
            }
            bool intersect(const ray& r, float* precomputedNumerator, float* precomputeDenominator, float& tNear, float& tFar, uint8_t& planeIndex) const;
            float d[kNumPlaneSetNormals][2];
        };

        Extents extents;
    public:
        BVH();
        BVH(const Mesh* mesh);
        BVH(const BVH& bvh);
        const bool intersect(const ray& ray, hitInfo& isectData) const;
        void computeBounds(const Mesh* mesh);
        ~BVH();

    };
}
