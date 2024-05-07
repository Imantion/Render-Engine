cbuffer ConstantBuffer : register(b0)
{
    row_major matrix projection;
}

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VOut main(float3 pos : POSITION, float4 color : COLOR)
{
    VOut output;

    output.position = mul(float4(pos, 1.0f), projection);
    output.color = color;

    return output;
}