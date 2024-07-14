#include "IBL.hlsli"
#include "..\Lights.hlsli"

TextureCube text : register(t0);

struct vsInput
{
    float4 pos : SV_POSITION;
    float2 textcoord : TEXTCOORD0;
};

float4 main(vsInput input) : SV_TARGET
{
    float2 rg = float2(0, 0);
    
    float3 normal = float3(0, 0, 1);
    float NoV = input.textcoord.y;
    float3 view = float3(sqrt(1 - NoV * NoV), 0, NoV);
    
    float roughness = input.textcoord.x;
    float roughness4 = pow(roughness, 4);

    int passedAmount = 0;
    
    for (int i = 0; i < g_numberOfSamples; i++)
    {
        float3x3 newBasis = basisFromDir(normal);
        float NoH;
        float h = randomGGX(NoH, i, g_numberOfSamples, roughness4, newBasis);
        float l = reflect(view, h);
        
        float NoL = dot(normal, l);
        float HoV = dot(view, h);
        
        if (NoL > 0.001f || HoV > 0.001f)
        {
            ++passedAmount;
            float G = G_Smith(roughness4, NoV, NoL);
            float denominator = NoV * NoL;
            float diff = pow((1 - HoV), 5);
            rg.r = G * (1 - diff) * HoV / denominator;
            rg.g = G * diff * HoV / denominator;
        }
    }

    rg /= passedAmount;
    
    return float4(rg, 0.0f, 1.0f);
}