#include "..\declarations.hlsli"

struct GSOutput
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3x3 tbn : TBN;
    float2 tc : TC;
    float3 spherePosition : SPHEREPOS;
    float3 particleColor : PARTICLECOLOR;
    float sphereRadius : SPHERERADIUS;
    float spherePreviousRadius : SPHEREPREVRADIUS;
    uint objectId : OBJECTID;
};

struct DSOut
{
    float3 worldPos : WorldPos;
    float3x3 tbn : TBN;
    float2 tc : TC;
    float3 spherePosition : SPHEREPOS;
    float3 particleColor : PARTICLECOLOR;
    float sphereRadius : SPHERERADIUS;
    float spherePreviousRadius : SPHEREPREVRADIUS;
    uint objectId : OBJECTID;
};


struct IncinerationParticle
{
    float3 position;
    float lifetime;
    float3 velocity;
    float passedTime;
    float3 irradiance;
};

cbuffer modelData : register(b4)
{
    float3 padding;
    float verteciesAmount;
}

RWStructuredBuffer<IncinerationParticle> particles : register(u5);
RWBuffer<uint> particleDataRange : register(u6);

[maxvertexcount(3)]
void main(
	triangle DSOut input[3],
    uint primitiveID: SV_PrimitiveID,
	inout TriangleStream<GSOutput> output
)
{
    float3 normal = normalize(cross(input[1].worldPos - input[0].worldPos, input[2].worldPos - input[0].worldPos));
    float3 center = (input[0].worldPos + input[1].worldPos + input[2].worldPos) / 3.0f;
    float3 toSphere = input[0].spherePosition - center;
    float squaredDistanceToSphere = dot(toSphere, toSphere);
    
    uint clipValue = max((uint)verteciesAmount / 3000u, 1);
      
    if (squaredDistanceToSphere < input[0].sphereRadius * input[0].sphereRadius &&
      squaredDistanceToSphere > input[0].spherePreviousRadius * input[0].spherePreviousRadius &&
        primitiveID % clipValue == 0)
    {
        uint index;
        InterlockedAdd(particleDataRange[0], 1u, index);
      
        index = (particleDataRange[1] + index) % MAX_PARTICLES;
          
        if (index >= MAX_PARTICLES)
            InterlockedAdd(particleDataRange[0], -1);
        else
        {
            particles[index].position = center;
            particles[index].velocity = normal * 2.0f;
            particles[index].irradiance = input[0].particleColor;
            particles[index].lifetime = 5.0f;
            particles[index].passedTime = 0.0f;
        }
    }
    
    for (uint i = 0; i < 3; i++)
    {
        GSOutput element;

        
        element.worldPos = input[i].worldPos;
        element.pos = mul(float4(element.worldPos, 1.0f), viewProjection);
        element.tbn = input[i].tbn;
        element.tc = input[i].tc;
        element.spherePosition = input[i].spherePosition;
        element.particleColor = input[i].particleColor;
        element.sphereRadius = input[i].sphereRadius;
        element.spherePreviousRadius = input[i].spherePreviousRadius;
        element.objectId = input[i].objectId;
        output.Append(element);
    }
}