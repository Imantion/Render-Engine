#include "..\GlobalSamplers.hlsli"
#include "..\declarations.hlsli"
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
    float3 finalColor = SpotLightContribution(input.normal, input.worldPos, g_cameraPosition);
    finalColor += PointLightContribution(input.normal, input.worldPos, g_cameraPosition);
    finalColor += DirectionalLightsContibution(input.normal, input.worldPos, g_cameraPosition);
    
    finalColor += ambient;
    float3 textureColor = text.Sample(g_anisotropicWrap, input.tc);
    
    return float4(textureColor * finalColor, 1.0f);
}