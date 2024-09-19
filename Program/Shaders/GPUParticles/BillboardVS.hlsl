#include "..\declarations.hlsli"

cbuffer textureInfo : register(b7)
{
    uint framesPerColumn;
    uint framesPerRow;
}
    

struct ParticleInstance
{
    uint vertexID : SV_VertexID;
    uint instanceID : SV_InstanceID;
};


struct VSOut
{
    float4 position : SV_Position;
    float4 emission : EMISSION;
    float2 tc : TEXTCOORD;
};

static const float2 offsets[4] =
{
    float2(-0.5f, -0.5f), // Bottom-left
    float2(0.5f, -0.5f), // Bottom-right
    float2(-0.5f, 0.5f), // Top-left
    float2(0.5f, 0.5f) // Top-right
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

VSOut main(ParticleInstance input)
{
    VSOut output;
    uint index = (dataRange[1] + input.instanceID) % MAX_PARTICLES;
    IncinerationParticle particle = particles[index];
    float3 cameraDir = normalize(g_cameraPosition - particle.position);
    float3 right = normalize(cross(float3(0.0f, 1.0f, 0.0f), cameraDir));
    float3 up = cross(cameraDir, right);
    float2 cornerOffset = (offsets[input.vertexID] + float2(0.0, 0.5)) * 0.025f;
    
    float3 worldPos = particle.position + (right * cornerOffset.x) + (up * cornerOffset.y);
    float interpolation = particles[index].passedTime / particles[index].lifetime;
    
    output.position = mul(float4(worldPos, 1.0f), viewProjection);
    output.emission = float4(particle.irradiance, lerp(1.0f, 0.05f, interpolation));
    output .tc = offsets[input.vertexID] + 0.5f;
    
    return output;
}