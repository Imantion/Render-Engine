#include "..\declarations.hlsli"

Texture2DMS<float> depthTexture : register(t19);

struct PSIn
{
    float4 pos : SV_POSITION;
    float2 textcord : TEXTCOORD0;
};

float main(PSIn input) : SV_Depth
{
    float maxDepth = 0.0f;
    for (int i = 0; i < g_samplesAmount; i++)
    {
        float textureDepth = depthTexture.Load(input.pos.xy, i);
        
        maxDepth = max(textureDepth, maxDepth);
    }

    return maxDepth;
}