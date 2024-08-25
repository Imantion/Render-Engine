#include "../declarations.hlsli"

struct VIn
{
    float4 decalToWorld[4] : TOWORLD;
    float4 worldToDecal[4] : TODECAL;
    uint objectID : OBJECTID;
    uint vertexID : SV_VertexID;
};

struct VOut
{
    float4 pos : SV_Position;
    nointerpolation float4x4 worldToDecal : TODECAL;
    nointerpolation uint objectID : OBJECTID;
};


static const float3 vertices[8] =
{
    // Front face
    { -0.5f, 0.5f, -0.5f }, // Top-left (0)
    { 0.5f, 0.5f, -0.5f }, // Top-right (1)
    { 0.5f, -0.5f, -0.5f }, // Bottom-right (2)
    { -0.5f, -0.5f, -0.5f }, // Bottom-left (3)

    // Back face
    { -0.5f, 0.5f, 0.5f }, // Top-left (4)
    { 0.5f, 0.5f, 0.5f }, // Top-right (5)
    { 0.5f, -0.5f, 0.5f }, // Bottom-right (6)
    { -0.5f, -0.5f, 0.5f }, // Bottom-left (7)
};


VOut main(VIn input)
{
    float4x4 decalToWorld = float4x4(input.decalToWorld[0], input.decalToWorld[1], input.decalToWorld[2], input.decalToWorld[3]);
    float4x4 worldToDecal = float4x4(input.worldToDecal[0], input.worldToDecal[1], input.worldToDecal[2], input.worldToDecal[3]);
    
    VOut output;
    output.pos = mul(float4(vertices[input.vertexID], 1.0f), decalToWorld);
    output.pos = mul(output.pos, viewProjection);
    output.worldToDecal = worldToDecal;
    output.objectID = input.objectID;
    
    return output;
}