Texture2D noiseTexture : register(t14);
SamplerState samp : register(s1);

struct PSIn
{
    float4 pos : SV_Position;
    float2 tc : TC;
    float duration : DURATION;
    float passedTime : TIMER;
};

float main(PSIn input) : SV_Depth
{
    float noise = noiseTexture.Sample(samp, input.tc);
    float dissolveTreshold = saturate(lerp(1.0f, 0.0f, input.passedTime / input.duration));
   
    float alpha = noise - dissolveTreshold;
    
    if(alpha < 0)
        discard;
    
    return input.pos.z;
}