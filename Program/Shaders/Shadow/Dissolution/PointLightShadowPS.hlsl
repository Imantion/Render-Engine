Texture2D noiseTexture : register(t14);
SamplerState samp : register(s1);

cbuffer linearDepthTransformData : register(b10)
{
    float3 lightPosition;
    float farPlane;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float2 tc : TC;
    float duration : DURATION;
    float passedTime : TIMER;
    uint renderTargetArrayIndex : SV_RenderTargetArrayIndex;
};


float main(PSInput input) : SV_DEPTH
{
    float noise = noiseTexture.Sample(samp, input.tc);
    float dissolveTreshold = saturate(lerp(1.0f, 0.0f, input.passedTime / input.duration));
   
    float alpha = noise - dissolveTreshold;
    
    if (alpha < 0)
        discard;
    
    float distance = length(lightPosition - input.worldPos);
    return 1.0f - distance / farPlane;
}