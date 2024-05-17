#include "declarations.hlsli"

struct DS_OUTPUT
{
    float3 vPosition : WORLDPOS;
    float3 normal : NORMAL;
};

struct GSOutput
{
    float4 pos : SV_Position;
    float3 vPos : WORLDPOS;
    float3 normal : NORMAL;
};

[maxvertexcount(6)]
void main(
	triangle DS_OUTPUT input[3] : SV_POSITION,
	inout LineStream<GSOutput> output
)
{
	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
		element.vPos = input[i].vPosition;
        element.pos = mul(float4(element.vPos, 1), viewProjection);
        element.normal = input[i].normal;
		output.Append(element);
        element.vPos = input[i].vPosition + input[i].normal;
        element.pos = mul(float4(element.vPos, 1), viewProjection);
        output.Append(element);
    }
}