#include "declarations.hlsli"

struct GSOutput
{
	float4 pos : SV_POSITION;
    float3 spacePosition : SPACEPOS;
    float3 normal : VERTEXNORMAL;
};

struct VOut
{
    float3 spacePosition : WORLDPOS;
    float3 normal : NORMAL;
    float offset : OFFSET;
};

[maxvertexcount(3)]
void main(
	triangle VOut input[3],
	inout TriangleStream< GSOutput > output
)
{
    float3 normal = normalize(cross(input[1].spacePosition - input[0].spacePosition, input[2].spacePosition - input[0].spacePosition));
	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
       
        element.spacePosition = input[i].spacePosition + normal * input[i].offset * 2.0f;
        element.pos = mul(float4(element.spacePosition,1.0f), viewProjection);
        element.normal = input[i].normal;
		output.Append(element);
	}
}