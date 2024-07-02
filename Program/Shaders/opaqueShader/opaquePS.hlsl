#include "..\declarations.hlsli"
#include "..\Lights.hlsli"

Texture2D text : register(t0);

struct PSInput
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3 normal : NORMAL;
    float2 tc : TC;
};

cbuffer materialData : register(b2)
{
    float roughness;
    float metallic;
}


float4 main(PSInput input) : SV_TARGET
{
    float3 albedo = text.Sample(g_anisotropicWrap, input.tc);
    
    float3 finalColor = float3(0, 0, 0);
    for (int i = 0; i < slSize; ++i)
    {
        float I = SpotLightCuttOffFactor(spotLights[i], input.worldPos, g_cameraPosition);
        float3 v = normalize(g_cameraPosition - input.worldPos);
        float3 l = spotLights[i].position - input.worldPos;
        float solidAngle = SolidAngle(spotLights[i].radiusOfCone, dot(l, l));
        l = normalize(l);
        finalColor += I * PBRLight(spotLights[i].color, solidAngle, l, albedo, metallic, roughness, input.normal, v);
    }
    for (i = 0; i < plSize; ++i)
    {
        float3 v = normalize(g_cameraPosition - input.worldPos);
        float3 l = pointLights[i].position - input.worldPos;
        float solidAngle = SolidAngle(pointLights[i].radius, dot(l, l));
        l = normalize(l);
        finalColor += PBRLight(pointLights[i].color, solidAngle, l, albedo, metallic, roughness, input.normal, v);
    }
    
    for (i = 0; i < dlSize; ++i)
    {
        float3 v = normalize(g_cameraPosition - input.worldPos);

        finalColor += PBRLight(directionalLights[i].color, directionalLights[i].solidAngle, -directionalLights[i].direction, albedo, metallic, roughness, input.normal, v);
    }
    finalColor += FlashLight(spotLights[0], albedo, metallic, roughness, input.normal, input.worldPos, g_cameraPosition);
   
    
    finalColor += ambient;
    
    
    return float4(finalColor, 1.0f);
}