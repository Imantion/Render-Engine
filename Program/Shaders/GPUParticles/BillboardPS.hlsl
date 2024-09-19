#include "..\declarations.hlsli"

struct PSIn
{
    float4 position : SV_Position;
    float4 emission : EMISSION;
    float2 tc : TEXTCOORD;

};

Texture2D<float4> sparkTexture : register(t4);

float4 main(PSIn input) : SV_TARGET
{
	float4 color = sparkTexture.Sample(g_sampler, input.tc);
    if(color.a < 0.1f)
        discard;
    input.emission.xyz *= color.xyz * 10;
    return input.emission;
}