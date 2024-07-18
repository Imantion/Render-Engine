#include "..\declarations.hlsli"
#include "..\Lights.hlsli"

#define MAX_MIP 10

Texture2D albed : register(t2);
Texture2D rough : register(t3);
Texture2D metal : register(t4);
Texture2D normalTexture : register(t5);

TextureCube diffuseIBL : register(t6);
TextureCube specIrrIBL : register(t7);
Texture2D reflectanceIBL : register(t8);
cbuffer MaterialData : register(b2)
{
    float material_flags;
    float material_roughness;
    float material_metalness;
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
    float3 normal = mul(((normalTexture.Sample(g_sampler, input.tc).rgb - 0.5f) * 2.0f), input.tbn);
    float3 v = normalize(g_cameraPosition - input.worldPos);
    
    float metalness = material_metalness;
    if(material_flags && 2)
        metalness = metal.Sample(g_sampler, input.tc).r;
    
    float roughness = material_roughness;
    if(material_flags && 1)
        roughness = rough.Sample(g_sampler, input.tc).r;
    
    
    if(input.isSelected)
    {
        if(input.shouldOverWriteMaterial)
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
        finalColor += I * PBRLight(spotLights[i].color, solidAngle, l, albedo, metalness, roughness, normal, v, specular, diffuse);
    }
    for (i = 0; i < plSize; ++i)
    {

        finalColor += PBRLight(pointLights[i], input.worldPos, albedo, metalness, roughness, input.tbn._31_32_33, normal, v, specular, diffuse);
    }
    
    for (i = 0; i < dlSize; ++i)
    {
        finalColor += PBRLight(directionalLights[i].color, directionalLights[i].solidAngle, -directionalLights[i].direction, albedo, metalness, roughness, normal, v, specular, diffuse);
    }
    finalColor += FlashLight(flashLight, albedo, metalness, roughness, normal, input.worldPos, g_cameraPosition, specular, diffuse);
   
    float2 refl = reflectanceIBL.Sample(g_linearWrap, float2(saturate(dot(normal, v)), roughness));
    float3 F0 = lerp(0.4f, albedo, metalness);
    
    if(IBL)
    finalColor += albedo * diffuseIBL.Sample(g_linearWrap, normal).rgb * (1 - metalness) + specIrrIBL.SampleLevel(g_linearWrap, normal, MAX_MIP * roughness).rgb * (refl.r * F0 + refl.g);
    
    
    return float4(finalColor, 1.0f);
}