#include "..\declarations.hlsli"

struct VIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 tc : TC;
    float4 modelToWorld[4] : TOWORLD;
};

struct VOut
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3 normal : NORMAL;
    float2 tc : TC;
};

cbuffer meshData : register(b2)
{
    float4x4 meshToModel;
}

VOut main(VIn input)
{
    float4x4 toWorld = float4x4(input.modelToWorld[0], input.modelToWorld[1], input.modelToWorld[2], input.modelToWorld[3]);
    VOut output;
    output.worldPos = mul(mul(float4(input.pos, 1.0f), meshToModel), toWorld);
    output.pos = mul(float4(output.worldPos,1.0f), viewProjection);
    output.normal = mul(float4(input.normal, 1.0f), meshToModel);
    output.normal = normalize(mul(output.normal, (float3x3) toWorld));
    output.tc = input.tc;
    
    return output;
}