#pragma once

#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <algorithm>
#include "Bone.h"
#include "assimp/anim.h"
#include "assimp/scene.h"
#include "Graphics/Model.h"

struct BoneInfo;


namespace Engine
{
    struct AssimpNodeData
    {
        mat4 transformation;
        std::string name;
        int childrenCount;
        std::vector<AssimpNodeData> children;
    };

    class Animation
    {
    public:

		Animation(const char* path, std::shared_ptr<Model> model);

        Bone* FindBone(const std::string& name);

        float GetTicksPerSecond() { return m_TicksPerSecond; }
        float GetDuration() { return m_duration; }
        const AssimpNodeData& GetRootNode() { return m_RootNode; }
        const std::map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }

    private:
        void ReadMissingBones(const aiAnimation* animation, Model& model);
        void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);

    private:
        float m_duration;
        int m_TicksPerSecond;
        std::vector<Bone> m_Bones;
        AssimpNodeData m_RootNode;
        std::map<std::string, BoneInfo> m_BoneInfoMap;
    };



    class Animator
    {
    public:
        Animator(Animation* animation);

        void UpdateAnimation(float dt);
        void PlayAnimation(Animation* pAnimation);
        std::vector<mat4> GetFinalBoneMatrices();

    private:
        void CalculateBoneTransform(const AssimpNodeData* node, mat4 parentTransform);

        std::vector<mat4> m_FinalBoneMatrices;
        Animation* m_CurrentAnimation;
        float m_CurrentTime;
        float m_DeltaTime;
    };
}
