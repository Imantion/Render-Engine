#include "..\GlobalSamplers.hlsli"
#include "..\Lights.hlsli"

Texture2D text : register(t0);

struct PSInput
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3 normal : NORMAL;
    float2 tc : TC;
};


float4 main(PSInput input) : SV_TARGET
{
    return text.Sample(g_anisotropicWrap, input.tc);
}