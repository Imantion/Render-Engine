#include "..\lights.hlsli"
#include "..\declarations.hlsli"
#include "IBL.hlsli"

TextureCube text : register(t0);


struct PSInput
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    uint renderTargetArrayIndex : SV_RenderTargetArrayIndex;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 color = float3(0, 0, 0);
    float3 normalizedNormal = normalize(input.normal);
    float3 view = normalizedNormal;
    int passedAmount = 0;
    
    float rouhgness4 = pow(g_roughness, 4.0f);
    float mipMultiplier = g_roughness == 0.0f ? 0.0f : 1.0f; // Because D_GGX returns 0 when roughness = 0. That's why miplevel should also be 0
    
    for (int i = 0; i < g_numberOfSamples; i++)
    {
        float3x3 newBasis = basisFromDir(normalizedNormal);
        float NoH;
        float3 h = randomGGX(NoH, i, g_numberOfSamples, rouhgness4, newBasis);
        
        float S = 4 / (2 * PI * D_GGX(rouhgness4, NoH) * g_numberOfSamples);
        float mip = hemisphereMip(S, g_resolution) * mipMultiplier;
        
        float3 l = normalize(2.0 * NoH * h - view);
        float NoL = dot(normalizedNormal, l);
        
        if(NoL > 0.0001f)
        {
            ++passedAmount;
            color += text.SampleLevel(g_anisotropicWrap, l, mip);
        }
    }

    color = color / passedAmount;
    
    return float4(color, 1.0f);
}