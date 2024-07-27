#include "..\lights.hlsli"
#include "..\declarations.hlsli"
#include "IBL.hlsli"

TextureCube text : register(t0);



struct PSInput
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    uint renderTargetArrayIndex : SV_RenderTargetArrayIndex;
};


float4 main(PSInput input) : SV_TARGET
{
    float3 color = float3(0, 0, 0);
    float3 normalizedNormal = normalize(input.normal);
    int passedAmount = 0;
    float mip = hemisphereMip(1.0F / g_numberOfSamples, g_resolution);
    for (int i = 0; i < g_numberOfSamples; i++)
    {
        float NoV;
        float3 sampleDirection = randomHemisphere(NoV, i, 4096);
        float3x3 newBasis = basisFromDir(normalizedNormal);
        sampleDirection = normalize(mul(sampleDirection, newBasis));
        float cos = dot(normalizedNormal, sampleDirection);
        if (NoV > 0.001f)
        {
            ++passedAmount;
            color += text.SampleLevel(g_anisotropicWrap, sampleDirection, mip).rgb * NoV / PI * (1 - fresnel(float3(0.04, 0.04, 0.04), NoV));
        }

    }
    color *= (2 * PI / passedAmount);
    
    return float4(color, 1);

}
