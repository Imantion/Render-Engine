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
    float3 spherePosition : SPHEREPOS;
    float sphereRadius : SPHERERADIUS;
    float spherePreviousRadius : SPHEREPREVRADIUS;
    uint renderTargetArrayIndex : SV_RenderTargetArrayIndex;
};

float main(PSInput input) : SV_DEPTH
{
    float3 toSphere = input.spherePosition - input.worldPos;
    float distanceToSphere = length(toSphere);
    
    if (distanceToSphere < input.sphereRadius)
    {
        float normedDistance = distanceToSphere / input.spherePreviousRadius;
        
        float alpha = noiseTexture.Sample(samp, input.tc).r;
        
        float dissolveThreshold = saturate(1.5f - normedDistance);
        
        if (alpha < dissolveThreshold)
        {
            discard;
        }
    }
    
    float distance = length(lightPosition - input.worldPos);
    return 1.0f - distance / farPlane;
}