#include "declarations.hlsli"

cbuffer frust : register(b2)
{
    float3 frusts[4];
};

struct psOutput
{
    float4 pos : SV_POSITION;
    float3 direction : TEXTCOORD0;
};

psOutput main(uint index : SV_VertexID)
{
    psOutput output;
    //float2 uv = float2((index << 1) & 2, index & 2);
    //float2 pos = uv * float2(2, -2) + float2(-1, 1);
    //float2 position[3] = { float2(-1, -1), float2(-1, 3) ,float2(3, -1)};
    float3 position[4] = { float3(-1, -1, 1), float3(-1, 1, 1), float3(1, -1, 1), float3(1, 1, 1) };
    
    output.pos = mul(float4(position[index], 0.0f), projection);
    output.pos.z = 0.0f;
    
    output.direction = mul(float4(position[index], 0), view);
    
    return output;
}