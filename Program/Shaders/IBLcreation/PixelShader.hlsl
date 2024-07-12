
cbuffer test : register(b3)
{
    float3 normal;
    float4 color;
};

float4 main() : SV_TARGET
{
	return color;
}