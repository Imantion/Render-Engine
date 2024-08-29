#include "..\declarations.hlsli"
#include "..\Lights.hlsli"

#define MAX_MIP 10


cbuffer MaterialData : register(b2)
{
    float material_flags;
    float material_roughness;
    float material_metalness;
}

cbuffer SLProjections : register(b5)
{
    float4x4 spotLightViewProejction[MAX_DL];
}

cbuffer DLProjections : register(b6)
{
    float4x4 dirLightViewProejction[MAX_DL];
}


struct PSInput
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3x3 tbn : TBN;
    float2 tc : TC;
    int isSelected : ISSELECTED;
    int shouldOverWriteMaterial : SHOULDOVERWRITE;
    float roughness : ROUGHNESS;
    float metalness : METALNESS;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 albedo = albed.Sample(g_sampler, input.tc);
    float3 normal = normalize(mul(((normalTexture.Sample(g_sampler, input.tc).rgb - 0.5f) * 2.0f), input.tbn));
    float3 macroNormal = input.tbn._31_32_33;
    float3 v = normalize(g_cameraPosition - input.worldPos);
    
    float metalness = material_metalness;
    if (material_flags && 2)
        metalness = metal.Sample(g_sampler, input.tc).r;
    
    float roughness = material_roughness;
    if (material_flags && 1)
        roughness = rough.Sample(g_sampler, input.tc).r;
    
    
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
    for (int i = 0; i < slSize; ++i)
    {
        float I = SpotLightCuttOffFactor(spotLights[i], input.worldPos, g_cameraPosition);
        
        float3 l = spotLights[i].position - input.worldPos;
        float solidAngle = SolidAngle(spotLights[i].radiusOfCone, dot(l, l));
        l = normalize(l);
        finalColor += I * PBRLight(spotLights[i].radiance, solidAngle, l, albedo, metalness, roughness, normal, v, specularState, diffuseState);
    }
    for (i = 0; i < plSize; ++i)
    {
        float3 lightDirection = input.worldPos - pointLights[i].position;
        float3 normalizedLightDirection = normalize(lightDirection);
        
        float depth = 1.0f - length(lightDirection - SHADOW_DEPTH_OFFSET * normalizedLightDirection) / g_PointLightFarPlane;
 
        float shadowValue = pointLightsShadowMap.SampleCmpLevelZero(compr, float4(normalizedLightDirection + macroNormal * 2.0f * depth / g_shadowResolution, i), depth).r;
        finalColor += shadowValue * PBRLight(pointLights[i], input.worldPos, albedo, metalness, roughness, input.tbn._31_32_33, normal, v, specularState, diffuseState);
    }
    
    for (i = 0; i < dlSize; ++i)
    {
        float3 textureUV = worldToUV(-directionalLights[i].direction + input.worldPos, input.worldPos, macroNormal, dirLightViewProejction[i]);
        float shadowValue = PCF(directionalLightsShadowMap, compr, i, textureUV, 1.0f / g_shadowResolution);
        finalColor += shadowValue * PBRLight(directionalLights[i].radiance, directionalLights[i].solidAngle, -directionalLights[i].direction, albedo, metalness, roughness, normal, v, specularState, diffuseState);
    }
    
   
    
    if (LTCState)
        for (i = 0; i < alSize; i++)
        {
            finalColor += LTC(areaLights[i], input.worldPos, normal, v, albedo, roughness, metalness);
        }

    
    float3 textureUV = worldToUV(flashLight.position, input.worldPos, macroNormal, spotLightViewProejction[slSize]);
    float shadowValue = spotLightsShadowMap.SampleCmpLevelZero(compr, float3(textureUV.xy, 0), textureUV.z + 0.000025f);
    finalColor += shadowValue * FlashLight(flashLight, albedo, metalness, roughness, normal, input.worldPos, g_cameraPosition, specularState, diffuseState);
   
    float2 refl = reflectanceIBL.Sample(g_sampler, float2(saturate(dot(normal, v)), roughness));
    float3 F0 = lerp(g_MIN_F0, albedo, metalness);
    if (IBLState)
        finalColor += albedo * diffuseIBL.Sample(g_sampler, normal).rgb * (1 - metalness) + specIrrIBL.SampleLevel(g_sampler, normal, MAX_MIP * roughness).rgb * (refl.r * F0 + refl.g);
    
    
    return float4(finalColor, 1.0f);
}