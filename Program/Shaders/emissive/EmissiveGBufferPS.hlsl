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
    float4 albedo : SV_Target0;
    float4 roughMetal : SV_Target1;
    float4 normals : SV_Target2;
    float4 emission : SV_Target3;
    uint objectId : SV_Target4;
    
};

PSOutput main(PSIn input)
{
    PSOutput output;
    
    output.albedo = float4(0, 0, 0, 1);
    output.roughMetal = float4(0, 0, 0, 1);
    
    float2 packedNormal = packOctahedron(input.normal);
    output.normals.xy = packedNormal;
    output.normals.zw = packedNormal;
    
    float3 cameraDir = normalize(g_cameraPosition - input.worldPos);
    float3 normedEmission = input.emission / max(input.emission.x,
	max(input.emission.y, max(input.emission.z, 1.0)));
    float3 normal = normalize(input.normal);
    float NoV = dot(cameraDir, normal);
    output.emission.xyz = lerp(normedEmission * 0.33, input.emission, pow(max(0.0, NoV), 8.0));
    output.emission.w = 1;
    
    output.objectId = input.objectId;
    
    return output;
}