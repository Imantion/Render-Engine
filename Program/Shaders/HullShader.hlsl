struct VS_CONTROL_POINT_OUTPUT
{
    float3 vPosition : WORLDPOS;
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

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT Output;

    Output.EdgeTessFactor[0] = abs(distance(ip[0].vPosition, ip[1].vPosition)) * 3;
    Output.EdgeTessFactor[1] = abs(distance(ip[1].vPosition, ip[2].vPosition)) * 3;
    Output.EdgeTessFactor[2] = abs(distance(ip[2].vPosition, ip[0].vPosition)) * 3;
    Output.InsideTessFactor = max(max(Output.EdgeTessFactor[0], Output.EdgeTessFactor[1]), Output.EdgeTessFactor[2]);
    
    //Output.EdgeTessFactor[0] = Output.EdgeTessFactor[1] = Output.EdgeTessFactor[2] = 1;
    //Output.InsideTessFactor = 0;

    return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONTROL_POINT_OUTPUT Output;

    Output.vPosition = ip[i].vPosition;
    Output.normal = ip[i].normal;
    Output.offset = ip[i].offset;

    return Output;
}
