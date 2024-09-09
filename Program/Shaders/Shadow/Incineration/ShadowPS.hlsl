Texture2D noiseTexture : register(t14);
SamplerState samp : register(s1);

struct PSIn
{
    float4 pos : SV_Position;
    float3 worldPos : WORLDPOS;
    float2 tc : TC;
    float3 spherePosition : SPHEREPOS;
    float sphereRadius : SPHERERADIUS;
    float spherePreviousRadius : SPHEREPREVRADIUS;
};

float main(PSIn input) : SV_Depth
{
    float3 toSphere = input.spherePosition - input.worldPos;
    float distance = length(toSphere);
    
    if (distance < input.sphereRadius)
    {
        float normedDistance = distance / input.spherePreviousRadius;
        
        float alpha = noiseTexture.Sample(samp, input.tc).r;
        
        float dissolveThreshold = saturate(1.5f - normedDistance);
        
        if (alpha < dissolveThreshold)
        {
            discard;
        }
    }
    
    return input.pos.z;
}