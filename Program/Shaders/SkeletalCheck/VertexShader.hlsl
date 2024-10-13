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
    nointerpolation int m_BoneIDs[MAX_BONE_INFLUENCE] : BONES;
    nointerpolation float m_Weights[MAX_BONE_INFLUENCE] : WEIGHTS;
    nointerpolation uint objectId : OBJECTID;
    
};

cbuffer meshData : register(b2)
{
    float4x4 meshToModel;
}

VOut main(VIn input)
{
    float4x4 toWorld = float4x4(input.modelToWorld[0], input.modelToWorld[1], input.modelToWorld[2], input.modelToWorld[3]);
    float3x3 normalizedToWorld = float3x3(normalize(input.modelToWorld[0].rgb), normalize(input.modelToWorld[1].rgb), normalize(input.modelToWorld[2].rgb));
    VOut output;
    output.worldPos = mul(mul(float4(input.pos, 1.0f), meshToModel), toWorld);
    output.pos = mul(float4(output.worldPos, 1.0f), viewProjection);
    
    float3x3 transformTBN = mul((float3x3) meshToModel, normalizedToWorld);
    float3 normal = normalize(mul(input.normal, transformTBN));
    float3 tangent = normalize(mul(input.tangent, transformTBN));
    float3 bitangent = normalize(mul(input.bitangent, transformTBN));
    
    output.tbn = float3x3(tangent, bitangent, normal);
    
    output.tc = input.tc;
    
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        output.m_BoneIDs[i] = input.m_BoneIDs[i];
        output.m_Weights[i] = input.m_Weights[i];
    }
    output.objectId = input.objectId;
    
    return output;
}