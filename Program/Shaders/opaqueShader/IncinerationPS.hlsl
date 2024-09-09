#include "opaque.hlsli"

struct PSInput
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3x3 tbn : TBN;
    float2 tc : TC;
    float3 spherePosition : SPHEREPOS;
    float3 particleColor : PARTICLECOLOR;
    float sphereRadius : SPHERERADIUS;
    float spherePreviousRadius : SPHEREPREVRADIUS;
    uint objectId : OBJECTID;
};

struct PSOutput
{
    float4 albedo : SV_Target0;
    float4 roughMetal : SV_Target1;
    float4 normals : SV_Target2;
    float4 emission : SV_Target3;
    uint objectId : SV_Target4;
};

Texture2D noiseTexture : register(t14);

PSOutput main(PSInput input) : SV_TARGET
{
    PSOutput output;
    float3 toSphere = input.spherePosition - input.worldPos;
    float distance = length(toSphere);
    
    float alpha = 1.0f;
    float glowFactor = 0.0f;
    if (distance < input.sphereRadius)
    {
        float normedDistance = distance / input.spherePreviousRadius;
        
        float alpha = noiseTexture.Sample(g_linearWrap, input.tc).r;
        
        float dissolveThreshold = saturate(1.5f - normedDistance);
        
        if (alpha < dissolveThreshold)
        {
            discard;
        }
        
        glowFactor = smoothstep(0.0f, 0.1f * input.sphereRadius, normedDistance);
        alpha = 1 - glowFactor;
    }
    
    output.albedo = float4(CalculateAlbedo(input.tc), alpha);
    CalculateMaterialProperties(input.tc, output.roughMetal.y, output.roughMetal.x);
    output.roughMetal.zw = float2(0, alpha);
    
    float3 microNormal = CalculateNormal(input.tc, input.tbn);

    output.normals.xy = packOctahedron(microNormal);
    output.normals.zw = packOctahedron(input.tbn._31_32_33);
    
    output.emission = float4(input.particleColor * glowFactor, 1.0f);
    output.objectId = input.objectId;
    
    return output;
}
