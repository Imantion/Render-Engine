#pragma pack_matrix(row_major)

cbuffer meshData : register(b2)
{
    float4x4 meshToModel;
}

cbuffer lightViewProjections : register(b4)
{
    float4x4 lightViewProjection;
};

struct VIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 tc : TC;
    float4 modelToWorld[4] : TOWORLD;
    float3 spherePosition : SPHEREPOS;
    float3 color : PARTICLECOLOR;
    float sphereRadius : SPHERERADIUS;
    float spherePreviousRadius : SPHEREPREVRADIUS;
    uint objectId : OBJECTID;
};

struct VOut
{
    float4 pos : SV_Position;
    float3 worldPos : WORLDPOS;
    float2 tc : TC;
    float3 spherePosition : SPHEREPOS;
    float sphereRadius : SPHERERADIUS;
    float spherePreviousRadius : SPHEREPREVRADIUS;
};


VOut main(VIn input)
{
    float4x4 toWorld = float4x4(input.modelToWorld[0], input.modelToWorld[1], input.modelToWorld[2], input.modelToWorld[3]);
   
    float3 worldPos = mul(mul(float4(input.pos, 1.0f), meshToModel), toWorld);
    VOut output;
    output.pos = mul(float4(worldPos, 1.0f), lightViewProjection);
    output.worldPos = worldPos;
    output.tc = input.tc;
    output.sphereRadius = input.sphereRadius;
    output.spherePreviousRadius = input.spherePreviousRadius;
    output.spherePosition = input.spherePosition + toWorld._41_42_43;

    return output;
}