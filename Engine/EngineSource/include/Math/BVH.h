//#include <Math/vec.h>
//#include "Mesh.h"
//#include <atomic>
//#include <memory>
//#include <cassert>
//#include <vector>
//#include <iostream>
//#include <fstream>
//#include <limits>
//#include <cmath>
//#include <chrono>
//#include <queue>
//using namespace Engine;
//
//const float kInfinity = std::numeric_limits<float>::max();
//
//class BBox
//{
//public:
//    BBox() {}
//    BBox(vec3 min_, vec3 max_)
//    {
//        bounds[0] = min_;
//        bounds[1] = max_;
//    }
//    BBox& extendBy(const vec3& p)
//    {
//        if (p.x < bounds[0].x) bounds[0].x = p.x;
//        if (p.y < bounds[0].y) bounds[0].y = p.y;
//        if (p.z < bounds[0].z) bounds[0].z = p.z;
//        if (p.x > bounds[1].x) bounds[1].x = p.x;
//        if (p.y > bounds[1].y) bounds[1].y = p.y;
//        if (p.z > bounds[1].z) bounds[1].z = p.z;
//
//        return *this;
//    }
//    /*inline */ vec3 centroid() const { return (bounds[0] + bounds[1]) * 0.5; }
//    vec3& operator [] (bool i) { return bounds[i]; }
//    const vec3 operator [] (bool i) const { return bounds[i]; }
//    bool intersect(const vec3&, const vec3&, const vec3&, float&) const;
//    vec3 bounds[2] = { kInfinity, -kInfinity };
//};
//
//
//bool BBox::intersect(const vec3& orig, const vec3& invDir, const vec3& sign, float& tHit) const
//{
//    float tmin, tmax, tymin, tymax, tzmin, tzmax;
//
//    tmin = (bounds[(int)sign[0]].x - orig.x) * invDir.x;
//    tmax = (bounds[(int)(1 - sign[0])].x - orig.x) * invDir.x;
//    tymin = (bounds[(int)sign[1]].y - orig.y) * invDir.y;
//    tymax = (bounds[(int)(1 - sign[1])].y - orig.y) * invDir.y;
//
//    if ((tmin > tymax) || (tymin > tmax))
//        return false;
//
//    if (tymin > tmin)
//        tmin = tymin;
//    if (tymax < tmax)
//        tmax = tymax;
//
//    tzmin = (bounds[(int)sign[2]].z - orig.z) * invDir.z;
//    tzmax = (bounds[(int)(1 - sign[2])].z - orig.z) * invDir.z;
//
//    if ((tmin > tzmax) || (tzmin > tmax))
//        return false;
//
//    if (tzmin > tmin)
//        tmin = tzmin;
//    if (tzmax < tmax)
//        tmax = tzmax;
//
//    tHit = tmin;
//
//    return true;
//}
//
//
//class AccelerationStructure
//{
//public:
// 
//    
//    AccelerationStructure(std::vector<std::unique_ptr<const Mesh>>& m) : meshes(std::move(m)) {}
//    virtual ~AccelerationStructure() {}
//    virtual bool intersect(const vec3& orig, const vec3& dir, const uint32_t& rayId, float& tHit) const
//    {
//        const Mesh* intersectedMesh = nullptr;
//        float t = kInfinity;
//        for (const auto& mesh : meshes) {
//            if (mesh->intersect(orig, dir, t) && t < tHit) {
//                intersectedMesh = mesh.get();
//                tHit = t;
//            }
//        }
//
//        return (intersectedMesh != nullptr);
//    }
//
//    const std::vector<std::unique_ptr<const Mesh>> meshes;
//};
//
