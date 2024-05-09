#include "declarations.hlsli"

float4 main(float4 fragCoord : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return color;
}

