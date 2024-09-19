struct VS_CONTROL_POINT_OUTPUT
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

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT Output;

    Output.EdgeTessFactor[0] = distance(ip[0].worldPos, ip[1].worldPos) * 3;
    Output.EdgeTessFactor[1] = distance(ip[1].worldPos, ip[2].worldPos) * 3;
    Output.EdgeTessFactor[2] = distance(ip[2].worldPos, ip[0].worldPos) * 3;
    
    Output.InsideTessFactor = (Output.EdgeTessFactor[0] + Output.EdgeTessFactor[1] + Output.EdgeTessFactor[2]) / 3.0f;

    return Output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONTROL_POINT_OUTPUT Output;

    Output.worldPos = ip[i].worldPos;
    Output.tangent = ip[i].tangent;
    Output.bitangent = ip[i].bitangent;
    Output.normal = ip[i].normal;
    Output.tc = ip[i].tc;
    Output.spherePosition = ip[i].spherePosition;
    Output.particleColor = ip[i].particleColor;
    Output.sphereRadius = ip[i].sphereRadius;
    Output.spherePreviousRadius = ip[i].spherePreviousRadius;
    Output.objectId = ip[i].objectId;

    return Output;
}
