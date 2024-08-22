#include "..\declarations.hlsli"

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 textcord : TEXTCOORD0;
};

Texture2D GBuffer_Albedo : register(t25);
Texture2D GBuffer_RouhgMetal : register(t26);
Texture2D GBuffer_Normals : register(t27);
Texture2D GBuffer_Emmision : register(t28);
Texture2D GBuffer_ObejctID : register(t29);
Texture2D DepthTexture : register(t30);

float4 main(PSInput input) : SV_TARGET
{
    return float4(GBuffer_Emmision.SampleLevel(g_linearWrap, input.textcord, 0).xyz, 1.0f);
}
