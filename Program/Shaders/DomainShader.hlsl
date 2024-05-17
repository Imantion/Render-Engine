#include "declarations.hlsli"

struct DS_OUTPUT
{
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float offset : OFFSET;
};

struct HS_CONTROL_POINT_OUTPUT
{
    float3 vPosition : WORLDPOS;
    float3 normal : NORMAL;
    float offset : OFFSET;
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
    Output.worldPos = patch[0].vPosition * domain.x + patch[1].vPosition * domain.y + patch[2].vPosition * domain.z;
    Output.normal = patch[0].normal * domain.x + patch[1].normal * domain.y + patch[2].normal * domain.z;
    Output.offset = (patch[0].offset + patch[1].offset + patch[2].offset) / 3.0f;
    return Output;
}
