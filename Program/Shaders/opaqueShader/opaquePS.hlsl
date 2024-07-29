#include "..\declarations.hlsli"
#include "..\Lights.hlsli"

Texture2D albed : register(t2);
Texture2D rough : register(t3);
Texture2D metal : register(t4);
Texture2D normalTexture : register(t5);


struct PSInput
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3x3 tbn : TBN;
    float2 tc : TC;
};



float4 main(PSInput input) : SV_TARGET
{
    float3 albedo = albed.Sample(g_sampler, input.tc);
    float metalness = metal.Sample(g_sampler, input.tc).r;
    float roughness = rough.Sample(g_sampler, input.tc).r;
    float3 normal = mul(((normalTexture.Sample(g_sampler, input.tc).rgb - 0.5f) * 2.0f), input.tbn);
    
    float3 finalColor = float3(0, 0, 0);
    for (int i = 0; i < slSize; ++i)
    {
        float I = SpotLightCuttOffFactor(spotLights[i], input.worldPos, g_cameraPosition);
        float3 v = normalize(g_cameraPosition - input.worldPos);
        float3 l = spotLights[i].position - input.worldPos;
        float solidAngle = SolidAngle(spotLights[i].radiusOfCone, dot(l, l));
        l = normalize(l);
        finalColor += I * PBRLight(spotLights[i].color, solidAngle, l, albedo, metalness, roughness, normal, v);
    }
    for (i = 0; i < plSize; ++i)
    {
        float3 v = normalize(g_cameraPosition - input.worldPos);
        float3 l = pointLights[i].position - input.worldPos;
        float solidAngle = SolidAngle(pointLights[i].radius, dot(l, l));
        l = normalize(l);
        finalColor += PBRLight(pointLights[i].color, solidAngle, l, albedo, metalness, roughness, normal, v);
    }
    
    for (i = 0; i < dlSize; ++i)
    {
        float3 v = normalize(g_cameraPosition - input.worldPos);

        finalColor += PBRLight(directionalLights[i].color, directionalLights[i].solidAngle, -directionalLights[i].direction, albedo, metalness, roughness, normal, v);
    }
    finalColor += FlashLight(flashLight, albedo, metalness, roughness, normal, input.worldPos, g_cameraPosition);
   
    
    finalColor += ambient;
    
    
    return float4(finalColor, 1.0f);
}