#include "..\declarations.hlsli"

struct PSin
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3 normal : NORMAL;
    nointerpolation float3 emission : EMISSION;
};

float4 main(PSin input) : SV_TARGET
{
    float3 normal = normalize(input.normal);
    float3 cameraDir = normalize(g_cameraPosition - input.worldPos);

    float3 normedEmission = input.emission / max(input.emission.x,
	max(input.emission.y, max(input.emission.z, 1.0)));

    float NoV = dot(cameraDir, normal);
    return float4(lerp(normedEmission * 0.33,
		input.emission, pow(max(0.0, NoV), 8.0)), 1.0f);

}