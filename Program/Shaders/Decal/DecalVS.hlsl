#include "../declarations.hlsli"

struct VIn
{
    float3 position : VERTEX;
    float4 decalToWorld[4] : TOWORLD;
    float4 worldToDecal[4] : TODECAL;
    uint objectID : OBJECTID;
    uint usedTextures : USEDTEXTURES;
    float roughness : ROUGHNESS;
    float metalness : METALNESS;
    float3 albedo : ALBEDO;
};
    
struct VOut
{
    float4 pos : SV_Position;
    nointerpolation float4x4 worldToDecal : TODECAL;
    nointerpolation uint objectID : OBJECTID;
    uint usedTextures : USEDTEXTURES;
    float roughness : ROUGHNESS;
    float metalness : METALNESS;
    float3 albedo : ALBEDO;
};


VOut main(VIn input)
{
    float4x4 decalToWorld = float4x4(input.decalToWorld[0], input.decalToWorld[1], input.decalToWorld[2], input.decalToWorld[3]);
    float4x4 worldToDecal = float4x4(input.worldToDecal[0], input.worldToDecal[1], input.worldToDecal[2], input.worldToDecal[3]);
    
    VOut output;
    output.pos = mul(float4(input.position, 1.0f), decalToWorld);
    output.pos = mul(output.pos, viewProjection);
    output.worldToDecal = worldToDecal;
    output.objectID = input.objectID;
    output.usedTextures = input.usedTextures;
    output.roughness = input.roughness;
    output.metalness = input.metalness;
    output.albedo = input.albedo;
    
    return output;
}