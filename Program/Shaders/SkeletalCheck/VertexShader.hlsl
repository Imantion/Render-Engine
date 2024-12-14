#include "..\declarations.hlsli"

struct VIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 tc : TC;
    int m_BoneIDs[MAX_BONE_INFLUENCE] : BONES;
    float m_Weights[MAX_BONE_INFLUENCE] : WEIGHTS;
    float4 modelToWorld[4] : TOWORLD;
    uint objectId : OBJECTID;
};

struct VOut
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3x3 tbn : TBN;
    float2 tc : TC;
    int m_BoneIDs[MAX_BONE_INFLUENCE] : BONES;
    float m_Weights[MAX_BONE_INFLUENCE] : WEIGHTS;
    nointerpolation uint objectId : OBJECTID;
    
};

cbuffer meshData : register(b2)
{
    float4x4 meshToModel;
}

cbuffer animData : register(b13)
{
    float4x4 animationTransform[128];
}


VOut main(VIn input)
{
    float4 totalPosition = float4(0.0f, 0.0f, 0.0f ,0.0f);
    float3 localNormal = input.normal;
    float3 localTangent = input.tangent;
    float3 localBitangent = input.bitangent;
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (input.m_BoneIDs[i] == -1) 
            continue;
        if (input.m_BoneIDs[i] >= 128)
        {
            totalPosition = float4(input.pos, 1.0f);
            break;
        }
        float4 localPosition = mul(float4(input.pos, 1.0f), animationTransform[input.m_BoneIDs[i]]);
        totalPosition += localPosition * input.m_Weights[i];

    }
    
    float4x4 toWorld = float4x4(input.modelToWorld[0], input.modelToWorld[1], input.modelToWorld[2], input.modelToWorld[3]);
    float3x3 normalizedToWorld = float3x3(normalize(input.modelToWorld[0].rgb), normalize(input.modelToWorld[1].rgb), normalize(input.modelToWorld[2].rgb));
    VOut output;
    output.worldPos = mul(mul(totalPosition, meshToModel), toWorld);
    //output.worldPos = mul(totalPosition, toWorld);
    output.pos = mul(float4(output.worldPos, 1.0f), viewProjection);
    
    float3x3 transformTBN = mul((float3x3) meshToModel, normalizedToWorld);
    float3 normal = normalize(mul(input.normal, transformTBN));
    float3 tangent = normalize(mul(input.tangent, transformTBN));
    float3 bitangent = normalize(mul(input.bitangent, transformTBN));
    
    output.tbn = float3x3(tangent, bitangent,normal);
    
    output.tc = input.tc;
    
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        output.m_BoneIDs[i] = input.m_BoneIDs[i];
        output.m_Weights[i] = input.m_Weights[i];
    }
    output.objectId = input.objectId;
    
    return output;
}