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
//cbuffer ConstantBuffer : register(b0)
//{
//    row_major matrix projection;
//}

//struct VIn
//{
//    float3 pos : POSITION;
//    float3 normal : NORMAL;
//    float3 tangent : TANGENT;
//    float3 bitangent : BITANGENT;
//    float2 tc : TC;
//};

//struct VOut
//{
//    float4 position : SV_POSITION;
//    float4 color : COLOR;
//};

//VOut main(VIn input)
//{
//    VOut output;

//    output.position = mul(float4(input.pos, 1.0f), projection);
//    output.color = output.position;

//    return output;
//}