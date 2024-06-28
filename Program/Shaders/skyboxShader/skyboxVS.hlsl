#include "..\declarations.hlsli"

cbuffer frust : register(b2)
{
    float4 frustCorners[3];
};

struct psOutput
{
    float4 pos : SV_POSITION;
    float3 direction : TEXTCOORD0;
};

static const float2 position[3] = { float2(-1.0f, -1.0f), float2(-1.0f, 3.0f), float2(3.0f, -1.0f) };

psOutput main(uint index : SV_VertexID)
{
    psOutput output;
    //float2 uv = float2((index << 1) & 2, index & 2);
    //float2 pos = uv * float2(2, -2) + float2(-1, 1);
    
    output.pos = float4(position[index],0.0f, 1.0f);
    
    output.direction = frustCorners[index];
    
    return output;
}