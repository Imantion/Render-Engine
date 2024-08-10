#pragma pack_matrix(row_major)

cbuffer meshData : register(b2)
{
    float4x4 meshToModel;
}

struct VIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 tc : TC;
    float4 modelToWorld[4] : TOWORLD;
    float duration : DURATION;
    float passedTime : TIMER;
};

struct VOut
{
    float3 worldPos : WORLDPOS;
    float2 tc : TC;
    float duration : DURATION;
    float passedTime : TIMER;
};


VOut main(VIn input)
{
    float4x4 toWorld = float4x4(input.modelToWorld[0], input.modelToWorld[1], input.modelToWorld[2], input.modelToWorld[3]);
    
    VOut output;
    output.worldPos = (float3)mul(mul(float4(input.pos, 1.0f), meshToModel), toWorld);
    output.tc = input.tc;
    output.duration = input.duration;
    output.passedTime = input.passedTime;
    
    return output;
}