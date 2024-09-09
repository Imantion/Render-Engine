#include "..\declarations.hlsli"

struct VIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 tc : TC;
    uint instanceID : SV_InstanceID;
};

struct VOut
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3 normal : NORMAL;
    nointerpolation float3 emission : EMISSION;
    nointerpolation float3 particlePos : PARTICLEPOS;
};

struct IncinerationParticle
{
    float3 position;
    float lifetime;
    float3 velocity;
    float passedTime;
    float3 irradiance;
};

StructuredBuffer<IncinerationParticle> particles : register(t4);
Buffer<uint> dataRange : register(t5);


VOut main(VIn input)
{
    uint index = (input.instanceID + dataRange[1]) % MAX_PARTICLES;
    VOut output;
    output.worldPos = input.pos * 0.1f + particles[index].position;
    output.pos = mul(float4(output.worldPos, 1.0f), viewProjection);
    output.normal = output.normal;
    
    float interpolation = 1 - particles[index].passedTime / particles[index].lifetime;
    
    output.emission = particles[index].irradiance * interpolation;
    output.particlePos = particles[index].position;
    
    return output;
}