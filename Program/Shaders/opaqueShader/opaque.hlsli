#ifndef OPAQUE_H
#define OPQUE_H

#include "../declarations.hlsli"
#include "Lights.hlsli"

cbuffer MaterialData : register(b2)
{
    float material_flags;
    float material_roughness;
    float material_metalness;
}

float3 CalculateNormal(float2 texCoord, float3x3 tbn)
{
    float3 normalMapValue = normalTexture.Sample(g_sampler, texCoord).rgb;
    float3 normal = normalize(mul((normalMapValue - 0.5f) * 2.0f, tbn));
    return normal;
}

float3 CalculateAlbedo(float2 texCoord)
{
    return albed.Sample(g_sampler, texCoord);
}

float4 CalculateAlbedo4(float2 texCoord)
{
    return albed.Sample(g_sampler, texCoord);
}

void CalculateMaterialProperties(float2 tc, out float metalness, out float roughness)
{
    metalness = material_metalness;
    if (material_flags && 2)
    {
        metalness = metal.Sample(g_sampler, tc).r;
    }

    roughness = material_roughness;
    if (material_flags && 1)
    {
        roughness = rough.Sample(g_sampler, tc).r;
    }
}

#endif
