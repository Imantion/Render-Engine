#include "..\declarations.hlsli"

Texture2D<float4> decalNormals : register(t24);

Texture2D<uint> idsTexutre : register(t25);
Texture2D<float4> normals : register(t26);
Texture2D<float> depth : register(t27);

struct PSIn
{
    float4 pos : SV_Position;
    nointerpolation float4x4 worldToDecal : TODECAL;
    nointerpolation uint objectID : OBJECTID;
};

struct PSOut
{
    float4 Albedo : SV_Target0;
    float4 RoughMetal : SV_Target1;
    float4 Normals : SV_Target2;
    float4 Emmisive : SV_Target3;
};

PSOut main(PSIn input)
{
    PSOut output;
    
    uint objectId = idsTexutre.Load(int3(input.pos.xy, 0));
    if (objectId != input.objectID)
        discard;
    
    float2 textCoord = (input.pos.xy - 0.5f) * float2(g_texelWidth, g_texelHeight);
    float2 clipSpace = (input.pos.xy - 0.5f) * float2(g_texelWidth, g_texelHeight);
    clipSpace.y = 1.0f - clipSpace.y;
    clipSpace = clipSpace * 2.0f - 1.0f;
    float4 pos = mul(float4(clipSpace, depth.Load(float3(input.pos.xy, 0)).x, 1.0f), inverseViewProjection);
    float3 worldPos = pos.xyz /= pos.w;
    float3 decalPos = mul(float4(worldPos, 1.0f), input.worldToDecal);
    clip(0.5f - abs(decalPos));
    
    float3 normal = unpackOctahedron(normals.Load(int3(input.pos.xy, 0)).zw);
    float3 DecalSpaceNormal = mul(normal, (float3x3) input.worldToDecal);
    if (DecalSpaceNormal.z > 0.0f)
        discard;
    
    float4 DecalNormal = decalNormals.SampleLevel(g_linearWrap, decalPos.xy + 0.5f, 0);
    
    output.Albedo = float4(0, 1, 1, DecalNormal.a);
    output.RoughMetal = float4(0.5, 0.5, 0, DecalNormal.a);
    output.Normals = float4(packOctahedron(normal), packOctahedron(normal));
    output.Emmisive = float4(0, 0, 0, 0);
    
    return output;
}

