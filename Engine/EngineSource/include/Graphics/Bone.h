#pragma once
#include <vector>
#include <string>
#include "Math/matrix.h"
#include "Math/quaternion.h"
#include "Math/vec.h"

struct aiNodeAnim;

namespace Engine
{
    struct KeyPosition
    {
        vec3 position;
        float timeStamp;
    };

    struct KeyRotation
    {
        quaternion orientation;
        float timeStamp;
    };

    struct KeyScale
    {
        vec3 scale;
        float timeStamp;
    };
}


namespace Engine
{
    class Bone
    {
    private:
        std::vector<KeyPosition> m_Positions;
        std::vector<KeyRotation> m_Rotations;
        std::vector<KeyScale> m_Scales;
        int m_NumPositions;
        int m_NumRotations;
        int m_NumScalings;

        mat4 m_LocalTransform;
        std::string m_Name;
        int m_ID;

    public:
        Bone(const std::string& name, int ID, const aiNodeAnim* channel);
        void Update(float animationTime);
        mat4 GetLocalTransform() { return m_LocalTransform; }
        std::string GetBoneName() const { return m_Name; }
        int GetBoneID() { return m_ID; }

    private:
        int GetPositionIndex(float animationTime);
        int GetRotationIndex(float animationTime);
        int GetScaleIndex(float animationTime);
        float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
        mat4 InterpolatePosition(float animationTime);
        mat4 InterpolateRotation(float animationTime);
        mat4 InterpolateScaling(float animationTime);
    };
}
