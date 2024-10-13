#include "Graphics/Bone.h"
#include "Graphics/Model.h"
#include "assimp/scene.h"
#include "Math/math.h"

namespace Engine
{
    Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
        : m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
    {
        // Read keyframes from aiNodeAnim
        m_NumPositions = channel->mNumPositionKeys;

        for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
        {
            aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
            float timeStamp = channel->mPositionKeys[positionIndex].mTime;
            KeyPosition data;
            data.position = reinterpret_cast<vec3&>(aiPosition);
            data.timeStamp = timeStamp;
            m_Positions.push_back(data);
        }

        m_NumRotations = channel->mNumRotationKeys;
        for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
        {
            aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
            float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
            KeyRotation data;
            data.orientation = reinterpret_cast<quaternion&>(aiOrientation);
            data.timeStamp = timeStamp;
            m_Rotations.push_back(data);
        }

        m_NumScalings = channel->mNumScalingKeys;
        for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
        {
            aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
            float timeStamp = channel->mScalingKeys[keyIndex].mTime;
            KeyScale data;
            data.scale = reinterpret_cast<vec3&>(scale);
            data.timeStamp = timeStamp;
            m_Scales.push_back(data);
        }
    }

    void Bone::Update(float animationTime)
    {
        mat4 translation = InterpolatePosition(animationTime);
        mat4 rotation = InterpolateRotation(animationTime);
        mat4 scale = InterpolateScaling(animationTime);
        m_LocalTransform = translation * rotation * scale;
    }

    int Bone::GetPositionIndex(float animationTime)
    {
        for (int index = 0; index < m_NumPositions - 1; ++index)
        {
            if (animationTime < m_Positions[index + 1].timeStamp)
                return index;
        }
        return m_NumPositions - 1; // Return last index if not found
    }

    int Bone::GetRotationIndex(float animationTime)
    {
        for (int index = 0; index < m_NumRotations - 1; ++index)
        {
            if (animationTime < m_Rotations[index + 1].timeStamp)
                return index;
        }
        return m_NumRotations - 1; // Return last index if not found
    }

    int Bone::GetScaleIndex(float animationTime)
    {
        for (int index = 0; index < m_NumScalings - 1; ++index)
        {
            if (animationTime < m_Scales[index + 1].timeStamp)
                return index;
        }
        return m_NumScalings - 1; // Return last index if not found
    }

    float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
    {
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff = nextTimeStamp - lastTimeStamp;
        return midWayLength / framesDiff;
    }

    mat4 Bone::InterpolatePosition(float animationTime)
    {
        mat4 translation = mat4::Identity();
        if (1 == m_NumPositions)
        {
            (vec3&)*translation[3] = m_Positions[0].position;
            return translation;
        }

        int p0Index = GetPositionIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp, m_Positions[p1Index].timeStamp, animationTime);
        vec3 finalPosition = lerp(m_Positions[p0Index].position, m_Positions[p1Index].position, scaleFactor);

        (vec3&)*translation[3] = finalPosition;
        return translation;
    }

    mat4 Bone::InterpolateRotation(float animationTime)
    {
        if (1 == m_NumRotations)
        {
            auto rotation = (m_Rotations[0].orientation).normalize();
            return rotation.toMat4();
        }

        int p0Index = GetRotationIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp, m_Rotations[p1Index].timeStamp, animationTime);
        quaternion finalRotation = quaternion::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation, scaleFactor);
        finalRotation = finalRotation.normalize();
        return finalRotation.toMat4();
    }

    mat4 Bone::InterpolateScaling(float animationTime)
    {
        mat4 scale = mat4::Identity();
        if (1 == m_NumScalings)
        {
            scale[0][0] = m_Scales[0].scale.x;
            scale[1][1] = m_Scales[0].scale.y;
            scale[2][2] = m_Scales[0].scale.z;
            return scale;
        }

        int p0Index = GetScaleIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp, m_Scales[p1Index].timeStamp, animationTime);
        vec3 finalScale = lerp(m_Scales[p0Index].scale, m_Scales[p1Index].scale, scaleFactor);
        scale[0][0] = finalScale.x;
        scale[1][1] = finalScale.y;
        scale[2][2] = finalScale.z;
        return scale;
    }
}
