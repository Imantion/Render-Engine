#include "opaque.hlsli"

struct PSInput
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3x3 tbn : TBN;
    float2 tc : TC;
    nointerpolation int isSelected : ISSELECTED;
    nointerpolation int shouldOverWriteMaterial : SHOULDOVERWRITE;
    nointerpolation float roughness : ROUGHNESS;
    nointerpolation float metalness : METALNESS;
};


float4 main(PSInput input) : SV_TARGET
{
    float3 albedo = CalculateAlbedo(input.tc);
    float3 normal = CalculateNormal(input.tc, input.tbn);
    float3 macroNormal = normalize(input.tbn._31_32_33);
    float3 viewDir = normalize(g_cameraPosition - input.worldPos);

    float metalness, roughness;
    CalculateMaterialProperties(input.tc, metalness, roughness);
    
    if (input.isSelected)
    {
        if (input.shouldOverWriteMaterial)
        {
            metalness = input.metalness;
            roughness = input.roughness;
        }
        else
        {
            metalness = saturate(metalness * input.metalness);
            roughness = saturate(roughness * input.roughness);
        }
    }

    float3 finalColor = float3(0, 0, 0);

    finalColor += CalculateSpotLightContribution(input.worldPos, albedo, metalness, roughness, normal, viewDir);
    finalColor += CalculatePointLightContribution(input.worldPos, albedo, metalness, roughness, normal, macroNormal, viewDir);
    finalColor += CalculateDirectionalLightContribution(input.worldPos, albedo, metalness, roughness, normal, macroNormal, viewDir);
    finalColor += CalculateAreaLightContribution(input.worldPos, albedo, metalness, roughness, normal, viewDir);
    finalColor += CalculateFlashLightContribution(input.worldPos, albedo, metalness, roughness, normal, macroNormal, viewDir);
    finalColor += CalculateIBLContribution(normal, albedo, metalness, roughness, viewDir);

    return float4(finalColor, 1.0f);
}
