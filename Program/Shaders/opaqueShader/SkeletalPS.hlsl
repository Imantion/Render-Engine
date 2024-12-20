#include "..\\declarations.hlsli"
#include "opaque.hlsli"

struct PSIn
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3x3 tbn : TBN;
    float2 tc : TC;
    nointerpolation uint objectId : OBJECTID;
    
};

cbuffer bone : register(b13)
{
    int selectedBoneId;
}

float4 main(PSIn input) : SV_TARGET
{
    float3 albedo = CalculateAlbedo(input.tc);
    float3 normal = CalculateNormal(input.tc, input.tbn);
    float3 macroNormal = normalize(input.tbn._31_32_33);
    float3 viewDir = normalize(g_cameraPosition - input.worldPos);

    float metalness, roughness;
    CalculateMaterialProperties(input.tc, metalness, roughness);
    
    float3 finalColor = float3(0, 0, 0);

    finalColor += CalculateSpotLightContribution(input.worldPos, albedo, metalness, roughness, normal, viewDir);
    finalColor += CalculatePointLightContribution(input.worldPos, albedo, metalness, roughness, normal, macroNormal, viewDir);
    finalColor += CalculateDirectionalLightContribution(input.worldPos, albedo, metalness, roughness, normal, macroNormal, viewDir);
    finalColor += CalculateAreaLightContribution(input.worldPos, albedo, metalness, roughness, normal, viewDir);
    finalColor += CalculateFlashLightContribution(input.worldPos, albedo, metalness, roughness, normal, macroNormal, viewDir);
    finalColor += CalculateIBLContribution(normal, albedo, metalness, roughness, viewDir);

    return float4(finalColor, 1.0f);
    
}