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

[maxvertexcount(2)]
void main(
	triangle DS_OUTPUT input[3],
	inout LineStream<GSOutput> output
)
{
    float3 vPos = input[0].vPosition;
    float3 normal = input[0].normal;
	for (uint i = 1; i < 3; i++)
	{
        vPos += input[i].vPosition;
        normal += input[i].normal;
    }
    vPos /= 3.0f;
    normal /= 3.0f;
    GSOutput element;
    element.vPos = vPos;
    element.normal = normalize(normal);
    element.pos = mul(float4(element.vPos, 1.0f), viewProjection);
    output.Append(element);
    
    element.vPos = vPos + element.normal * 0.05;
    element.pos = mul(float4(element.vPos, 1.0f), viewProjection);
    output.Append(element);
}