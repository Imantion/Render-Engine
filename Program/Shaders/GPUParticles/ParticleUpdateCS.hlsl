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

static const float3 GRAVITY_ACCELERATION = float3(0.0, -9.81, 0.0);

bool collision(float3 nextPos, float3 previousPos, out float3 normal, in out float distance)
{
    for (float i = 0; i <= 1.0f; i += 0.05f)
    {
        float3 interpolatedPos = lerp(previousPos, nextPos, i);
        
        float4 clipSpace = mul(float4(interpolatedPos, 1.0f), viewProjection);
        float3 NDC = clipSpace.xyz / clipSpace.w;
        NDC.xy = (NDC.xy + 1.0f) * 0.5f;
        NDC.y = 1.0f - NDC.y;
    
    
        if (!(all(NDC.xyz >= 0.0f) && all(NDC.xyz <= 1.0f)))
            continue;
        
        float depthValue = depth.SampleLevel(g_pointWrap, NDC.xy, 0);
        float3 objectProjected = float3(clipSpace.xy / clipSpace.w, depthValue);
        float4 inverseProjected = mul(float4(objectProjected, 1.0f), inverseViewProjection);
        float3 worldSpace = inverseProjected.xyz / inverseProjected.w;
        normal = unpackOctahedron(normalTexture.SampleLevel(g_pointWrap, NDC.xy, 0).zw);
        
        distance = length(worldSpace - interpolatedPos);
        
        if (depthValue > NDC.z && distance < 0.075f)
        {
            return true;
        }
    }

    return false;
}

bool SingleCollision(float3 pos, out float3 normal, inout float distance)
{   
    float4 clipSpace = mul(float4(pos, 1.0f), viewProjection);
    float3 NDC = clipSpace.xyz / clipSpace.w;
    NDC.xy = (NDC.xy + 1.0f) * 0.5f;
    NDC.y = 1.0f - NDC.y;
 
 
    if (!(all(NDC.xyz >= 0.0f) && all(NDC.xyz <= 1.0f)))
        return false;
     
    float depthValue = depth.SampleLevel(g_pointWrap, NDC.xy, 0);
    float3 objectProjected = float3(clipSpace.xy / clipSpace.w, depthValue);
    float4 inverseProjected = mul(float4(objectProjected, 1.0f), inverseViewProjection);
    float3 worldSpace = inverseProjected.xyz / inverseProjected.w;
    normal = unpackOctahedron(normalTexture.SampleLevel(g_pointWrap, NDC.xy, 0).zw);
     
    distance = length(worldSpace - pos);
     
    if (distance < 0.075f)
    {
        return true;
    }
    
    return false;
}


[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= particleDataRange[0])
    {
        return;
    }

    uint particleIndex = (index + particleDataRange[1]) % MAX_PARTICLES;
    IncinerationParticle particle = particles[particleIndex];
    
    float3 nextPos = particle.position;
    float3 velocity = particle.velocity;
    velocity -= 0.5f * 9.81f * g_deltaTime;
    nextPos += particle.velocity * g_deltaTime;
    velocity -= 0.5f * 9.81f * g_deltaTime;
    
    particle.passedTime += g_deltaTime;
    
    if (particle.passedTime >= particle.lifetime)
        InterlockedAdd(particleDataRange[2], 1);
    
    
    float3 normal;
    float distance;
    
    if (collision(nextPos, particle.position, normal, distance))
    {
        particle.velocity = reflect(particle.velocity, normal) * 0.325f;

        if (particle.passedTime < 0.5f)
        {
            particle.position += normal * distance * 2.0f;
        }

    }
    else
    {
        particle.position = nextPos;
    }
    
    particle.velocity.y -= 9.81f * g_deltaTime;
    
    particles[particleIndex] = particle;
}
