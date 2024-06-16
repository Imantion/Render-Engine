#include "..\declarations.hlsli"

struct VSOUT
{
    float4 pos : SV_POSITION;
    float2 textcord : TEXTCOORD0;
};

static const float2 position[4] = { float2(-1.0f, 1.0f), float2(1.0f, 1.0f), float2(-1.0f, -1.0f), float2(1.0f, -1.0f) };

VSOUT main(uint index : SV_VertexID)
{
    VSOUT vsOut;
    vsOut.pos = float4(position[index], 0.0f, 1.0f);
    vsOut.textcord = float2((position[index].x + 1.0f) / 2.0f, -(position[index].y - 1) / 2.0f);

    return vsOut;
}