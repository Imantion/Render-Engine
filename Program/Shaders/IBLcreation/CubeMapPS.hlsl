#include "..\lights.hlsli"

TextureCube text : register(t0);
SamplerState sampl : register(s0);

// Fibonacci hemisphere point uniform distribution
float3 randomHemisphere(out float NdotV, float i, float N)
{
    const float GOLDEN_RATIO = (1.0 + sqrt(5.0)) / 2.0;
    float theta = 2.0 * PI * i / GOLDEN_RATIO;
    float phiCos = NdotV = 1.0 - (i + 0.5) / N;
    float phiSin = sqrt(1.0 - phiCos * phiCos);
    float thetaCos, thetaSin;
    sincos(theta, thetaSin, thetaCos);
    return float3(thetaCos * phiSin, thetaSin * phiSin, phiCos);
}

// Frisvad with z == -1 problem avoidance
void basisFromDir(out float3 right, out float3 top, in float3 dir)
{
    float k = 1.0 / max(1.0 + dir.z, 0.00001);
    float a = dir.y * k;
    float b = dir.y * a;
    float c = -dir.x * a;
    right = float3(dir.z + b, c, -dir.x);
    top = float3(c, 1.0 - b, -dir.y);
}

// Frisvad with z == -1 problem avoidance
float3x3 basisFromDir(float3 dir)
{
    float3x3 rotation;
    rotation[2] = dir;
    basisFromDir(rotation[0], rotation[1], dir);
    return rotation;
}

float randomVanDeCorput(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// random2D from random1D
float2 randomHammersley(float i, float N)
{
    return float2(i / N, randomVanDeCorput(i));
}

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
    for (int i = 0; i < 10000; i++)
    {
        float NoV;
        float3 sampleDirection = randomHemisphere(NoV, i, 4096);
        float3x3 newBasis = basisFromDir(normalizedNormal);
        sampleDirection = normalize(mul(sampleDirection, newBasis));
        float cos = dot(normalizedNormal, sampleDirection);
        if (NoV > 0.001f)
        {
            ++passedAmount;
            color += text.SampleLevel(sampl, sampleDirection, 5).rgb * NoV / PI * (1 - fresnel(float3(0.04, 0.04, 0.04), NoV));
        }

    }
    color *= (2 * PI / passedAmount);
    
    return float4(color, 1);

}
