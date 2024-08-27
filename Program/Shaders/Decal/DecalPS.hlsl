#include "..\declarations.hlsli"

Texture2D<float4> decalNormals : register(t21);
Texture2D<float4> decalAlbedo : register(t22);
Texture2D<float4> decalRoughness : register(t23);
Texture2D<float4> decalMetalness : register(t24);

Texture2D<uint> idsTexutre : register(t25);
Texture2D<float4> normals : register(t26);
Texture2D<float> depth : register(t27);

struct PSIn
{
    float4 pos : SV_Position;
    nointerpolation float4x4 worldToDecal : TODECAL;
    nointerpolation uint objectID : OBJECTID;
    uint usedTextures : USEDTEXTURES;
    float roughness : ROUGHNESS;
    float metalness : METALNESS;
    float3 albedo : ALBEDO;
};

struct PSOut
{
    float4 Albedo : SV_Target0;
    float4 RoughMetal : SV_Target1;
    float4 Normals : SV_Target2;
    float4 Emmisive : SV_Target3;
};

// Frisvad with z == -1 problem avoidance
void basisFromDir(out float3 right, out float3 top, in float3 dir)
{
    float k = 1.0 / max(1.0 + dir.z, 0.00001);
    float a = dir.y * k;
    float b = dir.y * a;
    float c = -dir.x * a;
    right = float3(dir.z + b, c, -dir.x);
    top = float3(c, 1.0 - b, -dir.y);
}

// Frisvad with z == -1 problem avoidance
float3x3 basisFromDir(float3 dir)
{
    float3x3 rotation;
    rotation[2] = dir;
    basisFromDir(rotation[0], rotation[1], dir);
    return rotation;
}

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
    
    decalPos.xy += 0.5f;
    float4 DecalNormal = decalNormals.SampleLevel(g_linearWrap, decalPos.xy, 0);
    
    if (DecalNormal.a < 0.025f)
    {
        discard;
    }
    float3x3 TBN = basisFromDir(normal);
    float3 transformedDecalNormal = mul((float3) DecalNormal, TBN);
    
    float3 albedo = input.albedo;
    float2 roughMetal = float2(input.roughness, input.metalness);
    
    if (input.usedTextures & 1)
        roughMetal.x = decalRoughness.Sample(g_linearWrap, decalPos.xy).x;
    if (input.usedTextures & 2)
        roughMetal.y = decalMetalness.Sample(g_linearWrap, decalPos.xy).x;
    if (input.usedTextures & 4)
        albedo = decalAlbedo.Sample(g_linearWrap, decalPos.xy).xyz;

    
    output.Albedo = float4(albedo, DecalNormal.a);
    output.RoughMetal = float4(roughMetal, 0, DecalNormal.a);
    output.Normals = float4(packOctahedron(transformedDecalNormal), packOctahedron(normal));
    output.Emmisive = float4(0, 0, 0, 0);
    
    return output;
}


//PSOut main(PSIn input)
//{
//    PSOut output;
   

    
//    output.Albedo = float4(float3(1,0,1), 1);
//    output.RoughMetal = float4(0.5,0.5, 0, 1);
//    output.Normals = float4(packOctahedron(float3(1, 1, 1)), packOctahedron(float3(1, 1, 1)));
//    output.Emmisive = float4(0, 0, 0, 0);
    
//    return output;
//}

