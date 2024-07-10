#include "..\declarations.hlsli"

TextureCube text : register(t0);

struct psOutput
{
    float4 pos : SV_POSITION;
    float3 direction : TEXTCOORD0;
};


float4 main(psOutput direction) : SV_TARGET
{
    return text.Sample(g_pointWrap, direction.direction);
}