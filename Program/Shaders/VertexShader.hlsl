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

//#define CONCAT(a,b) a##b 
//#define bReg(index) CONCAT(b,index)

//cbuffer perView : register(b0)
//{
//    row_major matrix viewProjection;
//}

//cbuffer perFrame : register(b1)
//{
//    float4 iResolution;
//    float iTime;
//}

//cbuffer instanceBuffer : register(b3)
//{
//    row_major matrix transform;
//}

//cbuffer meshData : register(b4)
//{
//    row_major matrix meshToModel;
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

//    output.position = mul(float4(input.pos, 1.0f), viewProjection);
//    output.color = output.position;

//    return output;
//}