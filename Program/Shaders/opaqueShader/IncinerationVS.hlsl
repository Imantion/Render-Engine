#include "..\declarations.hlsli"

struct VIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 tc : TC;
    float4 modelToWorld[4] : TOWORLD;
    float3 spherePosition : SPHEREPOS;
    float3 particleColor : PARTICLECOLOR;
    float sphereRadius : SPHERERADIUS;
    float spherePreviousRadius : SPHEREPREVRADIUS;
    uint objectId : OBJECTID;
    uint vertexID : SV_VertexID;
};

struct VOut
{
    //float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 tc : TC;
    float3 spherePosition : SPHEREPOS;
    float3 particleColor : PARTICLECOLOR;
    float sphereRadius : SPHERERADIUS;
    float spherePreviousRadius : SPHEREPREVRADIUS;
    uint objectId : OBJECTID;
};

cbuffer meshData : register(b2)
{
    float4x4 meshToModel;
}

struct IncinerationParticle
{
    float3 position;
    float lifetime;
    float3 velocity;
    float passedTime;
    float3 irradiance;
};

//RWStructuredBuffer<IncinerationParticle> particles : register(u5);
//RWBuffer<uint> particleDataRange : register(u6);

VOut main(VIn input)
{
    float4x4 toWorld = float4x4(input.modelToWorld[0], input.modelToWorld[1], input.modelToWorld[2], input.modelToWorld[3]);
    float3x3 normalizedToWorld = float3x3(normalize(input.modelToWorld[0].rgb), normalize(input.modelToWorld[1].rgb), normalize(input.modelToWorld[2].rgb));
    VOut output;
    output.worldPos = mul(mul(float4(input.pos, 1.0f), meshToModel), toWorld);
    //output.pos = mul(float4(output.worldPos, 1.0f), viewProjection);
    
    float3x3 transformTBN = mul((float3x3) meshToModel, normalizedToWorld);
    float3 normal = normalize(mul(input.normal, transformTBN));
    float3 tangent = normalize(mul(input.tangent, transformTBN));
    float3 bitangent = normalize(mul(input.bitangent, transformTBN));
    
    //output.tbn = float3x3(tangent, bitangent, normal);
    
    output.tc = input.tc;
    
    //float3 toSphere = input.spherePosition - output.worldPos;
    //float squaredDistanceToSphere = dot(toSphere, toSphere);
        
    //if (squaredDistanceToSphere < input.sphereRadius * input.sphereRadius &&
    //    squaredDistanceToSphere > input.spherePreviousRadius * input.spherePreviousRadius && 
    //    input.vertexID % 200 == 0)
    //{
    //    uint index;
    //    InterlockedAdd(particleDataRange[0], 1u, index);
        
    //    index = (particleDataRange[1] + index) % MAX_PARTICLES;
            
    //    if(index >= MAX_PARTICLES)
    //        InterlockedAdd(particleDataRange[0], -1);
    //    else
    //    {
    //        particles[index].position = output.worldPos;
    //        particles[index].velocity = normal * 2.0f;
    //        particles[index].irradiance = input.particleColor;
    //        particles[index].lifetime = 5.0f;
    //        particles[index].passedTime = 0.0f;
    //    }
    //}
    
    output.spherePosition = input.spherePosition;
    output.sphereRadius = input.sphereRadius;
    output.spherePreviousRadius = input.spherePreviousRadius;
    output.objectId = input.objectId;
    output.particleColor = input.particleColor;
    output.normal = normal;
    output.tangent = tangent;
    output.bitangent = bitangent;
    
    return output;
}