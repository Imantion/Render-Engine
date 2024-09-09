#include "..\declarations.hlsli"

struct DS_OUTPUT
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

struct HS_CONTROL_POINT_OUTPUT
{
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

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
    DS_OUTPUT Output;
    Output.worldPos = patch[0].worldPos * domain.x + patch[1].worldPos * domain.y + patch[2].worldPos * domain.z;
    float3 normal = patch[0].normal * domain.x + patch[1].normal * domain.y + patch[2].normal * domain.z;
    float3 tangent = patch[0].tangent * domain.x + patch[1].tangent * domain.y + patch[2].tangent * domain.z;
    float3 bitangent = patch[0].bitangent * domain.x + patch[1].bitangent * domain.y + patch[2].bitangent * domain.z;
    Output.tbn = float3x3(tangent, bitangent, normal);
    Output.tc = patch[0].tc * domain.x + patch[1].tc * domain.y + patch[2].tc * domain.z;
    Output.spherePosition = patch[0].spherePosition;
    Output.particleColor = patch[0].particleColor;
    Output.sphereRadius = patch[0].sphereRadius;
    Output.spherePreviousRadius = patch[0].spherePreviousRadius;
    Output.objectId = patch[0].objectId;
    return Output;
}
