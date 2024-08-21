#include "..\declarations.hlsli"

struct psOutput
{
    float4 pos : SV_POSITION;
    float3 direction : TEXTCOORD0;
};

static const float2 position[3] = { float2(-1.0f, -1.0f), float2(-1.0f, 3.0f), float2(3.0f, -1.0f) };

psOutput main(uint index : SV_VertexID)
{
    psOutput output;
    
    output.pos = float4(position[index],0.0f, 1.0f);
    
    output.direction = bottomLeft + (position[index].x + 1) * 0.5 * rightVector + (position[index].y + 1) * 0.5 * topVector;;
    
    return output;
}