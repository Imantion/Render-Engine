#include "..\declarations.hlsli"

Texture2D depthTexture : register(t19);

struct PSIn
{
    float4 pos : SV_POSITION;
    float2 textcord : TEXTCOORD0;
};

float main(PSIn input) : SV_Depth
{
    return depthTexture.Sample(g_linearWrap, input.textcord);
}