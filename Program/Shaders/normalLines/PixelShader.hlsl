#include "../declarations.hlsli"

struct VOut
{
    float4 position : SV_POSITION;
    float3 vPosition : WORLDPOS;
    float3 normal : NORMAL;
};

float4 main(VOut input) : SV_TARGET
{   
    return float4(1.0f,1.0f,0.0f, 1.0f);
}

