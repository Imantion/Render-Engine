#include "opaque.hlsli"

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 textcord : TEXTCOORD0;
};

Texture2D<float3> GBuffer_Albedo : register(t25);
Texture2D<float2> GBuffer_RouhgMetal : register(t26);
Texture2D<float4> GBuffer_Normals : register(t27);
Texture2D<float3> GBuffer_Emmision : register(t28);
Texture2D<uint> GBuffer_ObejctID : register(t29);
Texture2D<float> DepthTexture : register(t30);

float4 main(PSInput input) : SV_TARGET
{
    float3 albedo = GBuffer_Albedo.SampleLevel(g_linearWrap, input.textcord, 0);
    float2 roughmetal = GBuffer_RouhgMetal.SampleLevel(g_linearWrap, input.textcord, 0);
    float roughness = roughmetal.x;
    float metalness = roughmetal.y;
    float3 normal = unpackOctahedron(GBuffer_Normals.SampleLevel(g_linearWrap, input.textcord, 0).xy);
    float3 macroNormal = unpackOctahedron(GBuffer_Normals.SampleLevel(g_linearWrap, input.textcord, 0).zw);
    float2 clip = input.pos.xy / float2(800, 400);
    clip.y = 1.0f - clip.y;
    clip = clip * 2.0f - 1.0f;
    float4 pos = mul(float4(clip, DepthTexture.SampleLevel(g_linearWrap, input.textcord, 0).x, 1.0f), inverseProjection);
    pos /= pos.w;
    float3 worldPos = mul(pos, inverseView);
    float3 viewDir = normalize(g_cameraPosition - worldPos);
    float3 emission = GBuffer_Emmision.SampleLevel(g_linearWrap, input.textcord, 0);

    float3 finalColor = float3(0, 0, 0);

    finalColor += CalculateSpotLightContribution(worldPos, albedo, metalness, roughness, normal, viewDir);
    finalColor += CalculatePointLightContribution(worldPos, albedo, metalness, roughness, normal, macroNormal, viewDir);
    finalColor += CalculateDirectionalLightContribution(worldPos, albedo, metalness, roughness, normal, macroNormal, viewDir);
    finalColor += CalculateAreaLightContribution(worldPos, albedo, metalness, roughness, normal, viewDir);
    finalColor += CalculateFlashLightContribution(worldPos, albedo, metalness, roughness, normal, macroNormal, viewDir);
    finalColor += CalculateIBLContribution(normal, albedo, metalness, roughness, viewDir);
    finalColor += emission;

    return float4(finalColor, 1.0f);
}
