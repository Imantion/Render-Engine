#include "opaque.hlsli"

struct PSInput
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3x3 tbn : TBN;
    float2 tc : TC;
    nointerpolation float duration : DURATION;
    nointerpolation float passedTime : TIMER;
    nointerpolation uint objectId : OBJECTID;
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

PSOutput main(PSInput input)
{
    PSOutput output;
    
    float noise = noiseTexture.Sample(g_linearWrap, input.tc);
    float dissolveTreshold = saturate(lerp(1.0f, 0.0f, input.passedTime / input.duration));
 
    float alpha = noise - dissolveTreshold;
    float glow = saturate((0.025 - alpha) / 0.025);
    
    if(alpha < 0.0f)
        discard;
    
    alpha = saturate((alpha / (abs(ddx(noise)) + abs(ddy(noise)))) / 0.05f);
    output.albedo = float4(CalculateAlbedo(input.tc), alpha);
    CalculateMaterialProperties(input.tc, output.roughMetal.y, output.roughMetal.x);
    output.roughMetal.zw = float2(0, 1);
    
    float3 microNormal = CalculateNormal(input.tc, input.tbn);

    output.normals.xy = packOctahedron(microNormal);
    output.normals.zw = packOctahedron(input.tbn._31_32_33);
    
    output.emission = float4(glow * float3(100, 100, 100), 1);
    output.objectId = input.objectId;
    
    return output;
}