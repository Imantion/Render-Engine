#include "..\declarations.hlsli"
struct PSIn
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3 normal : NORMAL;
    nointerpolation float3 emission : EMISSION;
    nointerpolation uint objectId : OBJECTID;
};

struct PSOutput
{
    float3 albedo : SV_Target0;
    float2 roughMetal : SV_Target1;
    float4 normals : SV_Target2;
    float3 emission : SV_Target3;
    uint objectId : SV_Target4;
    
};

PSOutput main(PSIn input)
{
    PSOutput output;
    
    output.albedo = float3(0, 0, 0);
    output.roughMetal = float2(0, 0);
    
    float2 packedNormal = packOctahedron(input.normal);
    output.normals.xy = packedNormal;
    output.normals.zw = packedNormal;
    
    output.emission = input.emission;
    output.objectId = input.objectId;
    
    return output;
}