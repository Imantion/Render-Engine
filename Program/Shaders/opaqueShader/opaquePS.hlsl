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
    float3 finalColor = float3(0, 0, 0);
    for (int i = 0; i < slSize; ++i)
    {
        finalColor += SpotLightContribution(spotLights[i], input.normal, input.worldPos, g_cameraPosition);
    }
    for (i = 0; i < plSize; ++i)
    {
        finalColor += PointLightContribution(pointLights[i],input.normal, input.worldPos, g_cameraPosition);
    }
    
    for (i = 0; i < dlSize; ++i)
    {
        finalColor += DirectionalLightsContibution(directionalLights[i], input.normal, input.worldPos, g_cameraPosition);
    }
    finalColor += FlashLight(flashLight, input.normal, input.worldPos, g_cameraPosition);
   
    
    finalColor += ambient;
    float3 textureColor = text.Sample(g_anisotropicWrap, input.tc);
    
    return float4(textureColor * finalColor, 1.0f);
}