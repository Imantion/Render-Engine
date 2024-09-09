#include "..\declarations.hlsli"

struct PSIn
{
    float4 position : SV_Position;
    float3 emission : EMISSION;
    float2 tc : TEXTCOORD;
};

Texture2D<float4> sparkTexture : register(t4);

float4 main(PSIn input) : SV_TARGET
{
	float4 color = sparkTexture.Sample(g_sampler, input.tc);
    if(color.a < 0.1f)
        discard;
    return float4(color.xyz * input.emission * 10, color.a);
}