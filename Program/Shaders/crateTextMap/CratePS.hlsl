#include "..\declarations.hlsli"

Texture2D text : register(t0);

struct PSIn
{
    float4 pos : SV_Position;
    float2 tc : TC;
};

float4 main(PSIn input) : SV_TARGET
{
    return text.Sample(g_sampler, input.tc);
}