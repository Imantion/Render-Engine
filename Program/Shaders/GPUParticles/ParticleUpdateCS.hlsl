#include "../declarations.hlsli"

struct IncinerationParticle
{
    float3 position;
    float lifetime;
    float3 velocity;
    float passedTime;
    float3 irradiance;
};

RWStructuredBuffer<IncinerationParticle> particles : register(u0);
RWBuffer<uint> particleDataRange : register(u1);

Texture2D<float4> normalTexture : register(t0);
Texture2D<float> depth : register(t1);

[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint index = DTid.x;
    if (index >= particleDataRange[0])
    {
        return;
    }

    uint particleIndex = (index + particleDataRange[1]) % MAX_PARTICLES;
    IncinerationParticle particle = particles[particleIndex];
    particle.velocity.y -= 0.5f * 9.81f * g_deltaTime ;
    particle.position += particle.velocity * g_deltaTime;
    particle.velocity.y -= 0.5f * 9.81f * g_deltaTime;
    
    
    particle.passedTime += g_deltaTime;
   
    
    if(particle.passedTime >= particle.lifetime)
        InterlockedAdd(particleDataRange[2], 1);
    
    float4 clipSpace = mul(float4(particle.position, 1.0f), viewProjection);
    float3 NDC = clipSpace.xyz / clipSpace.w;
    NDC.xy = (NDC.xy + 1.0f) * 0.5f;
    NDC.y = 1.0f - NDC.y;
    
    if (all(NDC.xyz >= 0.0f) && all(NDC.xyz <= 1.0f))
    {
        float depthValue = depth.SampleLevel(g_pointWrap, NDC.xy, 0);
        float3 objectProjected = float3(clipSpace.xy / clipSpace.w, depthValue);
        float4 inverseProjected = mul(float4(objectProjected, 1.0f), inverseViewProjection);
        float3 worldSpace = inverseProjected.xyz / inverseProjected.w;
        float3 normal = unpackOctahedron(normalTexture.SampleLevel(g_pointWrap, NDC.xy, 0).zw);
        
        float distance = normal.x * (particle.position.x - worldSpace.x) + normal.y * (particle.position.y - worldSpace.y) + normal.z * (particle.position.z - worldSpace.z);
        if(distance < 0.0f && distance > -0.05f)
        {
            particle.velocity.xyz = reflect(particle.velocity.xyz, normal) * 0.5f;
            if(distance < -0.015f)
            {
                particle.position = worldSpace + normal * 0.025f;
            }

            particle.position += particle.velocity * g_deltaTime;
        }
     
    }
    
    particles[particleIndex] = particle;
}
