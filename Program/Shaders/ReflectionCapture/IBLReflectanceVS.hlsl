struct vsOutput
{
    float4 pos : SV_POSITION;
    float2 textcoord : TEXTCOORD0;
};

static const float2 position[3] = { float2(-1.0f, -1.0f), float2(-1.0f, 3.0f), float2(3.0f, -1.0f) };

vsOutput main(uint index : SV_VertexID)
{
    vsOutput output;
    
    output.pos = float4(position[index], 0.0f, 1.0f);
    
    output.textcoord = (position[index] + 1.0f) * 0.5f;
    
    return output;
}