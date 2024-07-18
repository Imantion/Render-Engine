#include "..\declarations.hlsli"

struct VIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 tc : TC;
    float4 modelToWorld[4] : TOWORLD;
    int isSelected : ISSELECTED;
    int shouldOverWriteMaterial : SHOULDOVERWRITE;
    float roughness : ROUGHNESS;
    float metalness : METALNESS;
};

struct VOut
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3x3 tbn : TBN;
    float2 tc : TC;
    int isSelected : ISSELECTED;
    int shouldOverWriteMaterial : SHOULDOVERWRITE;
    float roughness : ROUGHNESS;
    float metalness : METALNESS;
    
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
    output.pos = mul(float4(output.worldPos,1.0f), viewProjection);
    
    float3x3 transformTBN = mul((float3x3) meshToModel, normalizedToWorld);
    float3 normal = normalize(mul(input.normal, transformTBN));
    float3 tangent = normalize(mul(input.tangent, transformTBN));
    float3 bitangent = normalize(mul(input.bitangent, transformTBN));
    
    output.tbn = float3x3(tangent, bitangent, normal);
    
    output.tc = input.tc;
    
    output.isSelected = input.isSelected;
    output.shouldOverWriteMaterial = input.shouldOverWriteMaterial;
    output.roughness = input.roughness;
    output.metalness = input.metalness;
    
    return output;
}