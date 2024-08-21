
static const float2 position[3] = { float2(-1.0f, -1.0f), float2(-1.0f, 3.0f), float2(3.0f, -1.0f) };

float4 main( uint index : SV_VertexID ) : SV_POSITION
{
    return float4(position[index], 0.0f, 1.0f);
}