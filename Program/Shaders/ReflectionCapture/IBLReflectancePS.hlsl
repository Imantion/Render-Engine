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
    float NoV = input.textcoord.x;
    float3 view;
    view.x = sqrt(1 - NoV * NoV);
    view.y = 0;
    view.z = NoV;
   
    float roughness = input.textcoord.y;
    float roughness4 = pow(roughness, 4);

    int passedAmount = 0;
    
    for (int i = 0; i < g_numberOfSamples; i++)
    {
        float3x3 newBasis = basisFromDir(normal);
        float NoH;
        float3 h = normalize(randomGGX(NoH, i, g_numberOfSamples, roughness4, newBasis));
        float HoV = dot(h, view);
        float3 l = normalize(2.0 * HoV * h - view);
        float NoL = l.z;
       
        if (NoL > 0.0f && HoV > 0.0f)
        {
            ++passedAmount;
            float G = G_Smith(roughness4, NoV, NoL);
            float mult = G * HoV / (NoV * NoH);
            float F = pow((1 - HoV), 5);
            rg.r += (1 - F) * mult;
            rg.g += F * mult;
        }
    }

    rg /= passedAmount;
    
    return float4(rg, 0.0f, 1.0f);
}