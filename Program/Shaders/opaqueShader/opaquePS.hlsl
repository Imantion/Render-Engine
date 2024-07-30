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
Texture2D LTCmat : register(t9);
Texture2D LTCamp : register(t10);
cbuffer MaterialData : register(b2)
{
    float material_flags;
    float material_roughness;
    float material_metalness;
}

TextureCubeArray pointLightsShadowMap : register(t11);

SamplerComparisonState compr : register(s5);


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

// Define the four corners of a rectangular area light
static const float3 areaLightPoints[4] =
{
    float3(-1.0, 1.0, 0.0), // Top left
    float3(1.0, 1.0, 0.0), // Top right
    float3(1.0, -1.0, 0.0), // Bottom right
    float3(-1.0, -1.0, 0.0) // Bottom left
};


float4 main(PSInput input) : SV_TARGET
{
    float3 albedo = albed.Sample(g_sampler, input.tc);
    float3 normal = normalize(mul(((normalTexture.Sample(g_sampler, input.tc).rgb - 0.5f) * 2.0f), input.tbn));
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
        finalColor += I * PBRLight(spotLights[i].color, solidAngle, l, albedo, metalness, roughness, normal, v, specularState, diffuseState);
    }
    for (i = 0; i < plSize; ++i)
    {
        float3 directionToLigt = input.worldPos - pointLights[i].position;
        float depth = 1.0f - length(directionToLigt) / 100.0f + 0.005f;
        float shadowValue = pointLightsShadowMap.SampleCmp(compr, float4(directionToLigt, i), depth).r;
   
        finalColor += shadowValue * PBRLight(pointLights[i], input.worldPos, albedo, metalness, roughness, input.tbn._31_32_33, normal, v, specularState, diffuseState);
    }
    
    for (i = 0; i < dlSize; ++i)
    {
        finalColor += PBRLight(directionalLights[i].color, directionalLights[i].solidAngle, -directionalLights[i].direction, albedo, metalness, roughness, normal, v, specularState, diffuseState);
    }
    
    float dotNV = clamp(dot(normal, v), 0.0f, 1.0f);

    // use roughness and sqrt(1-cos_theta) to sample M_texture
    float2 uv = float2(roughness, sqrt(1.0f - dotNV));
    uv = uv * LUT_SCALE + LUT_BIAS;

    float4 t1 = LTCmat.Sample(g_linearWrap, uv);
    float t2 = LTCamp.Sample(g_linearWrap, uv);
    
    float3x3 Minv = float3x3(
    float3(t1.x, 0, t1.y),
    float3(0, 1, 0),
    float3(t1.z, 0, t1.w)
    );
    
    float3x3 Identity =
    {
        { 1, 0, 0, },
        { 0, 1, 0, },
        { 0, 0, 1 },
    };
    
    if (LTCState)
        for (i = 0; i < alSize; i++)
        {
            float3 d = LTC_Evaluate(normal, v, input.worldPos, Identity, areaLights[i], true);
            float3 s = LTC_Evaluate(normal, v, input.worldPos, Minv, areaLights[i], true);
            finalColor += areaLights[i].color * (d * albedo * (1 - metalness) + s) * (t2.r * areaLights[i].intensity);
        }

    
    finalColor += FlashLight(flashLight, albedo, metalness, roughness, normal, input.worldPos, g_cameraPosition, specularState, diffuseState);
   
    float2 refl = reflectanceIBL.Sample(g_sampler, float2(saturate(dot(normal, v)), roughness));
    float3 F0 = lerp(g_MIN_F0, albedo, metalness);
    
    if (IBLState)
        finalColor += albedo * diffuseIBL.Sample(g_sampler, normal).rgb * (1 - metalness) + specIrrIBL.SampleLevel(g_sampler, normal, MAX_MIP * roughness).rgb * (refl.r * F0 + refl.g);
    
    
    return float4(finalColor, 1.0f);
}