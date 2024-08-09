#include "opaque.hlsli"

Texture2D noiseTexture : register(t14);

struct PSInput
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3x3 tbn : TBN;
    float2 tc : TC;
    float duration : DURATION;
    float passedTime : TIMER;
}; 

float4 main(PSInput input) : SV_TARGET
{
    float noise = noiseTexture.Sample(g_linearWrap, input.tc);
    float dissolveTreshold = saturate(lerp(1.0f, 0.0f, input.passedTime / input.duration));
   
    float alpha = noise - dissolveTreshold;
    float glow = saturate((0.05 - alpha) / 0.05);
   
   
    
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
    
    finalColor = (1 - glow) * finalColor + float3(100, 100, 100) * glow;
    

    alpha = saturate((alpha / (abs(ddx(noise)) + abs(ddy(noise)))) / 0.05f);
    return float4(finalColor, alpha);
}
