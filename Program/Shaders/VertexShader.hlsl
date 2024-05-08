//cbuffer ConstantBuffer : register(b0)
//{
//    row_major matrix projection;
//}

//struct VOut
//{
//    float4 position : SV_POSITION;
//    float4 color : COLOR;
//};

//VOut main(float3 pos : POSITION, float4 color : COLOR)
//{
//    VOut output;

//    output.position = mul(float4(pos, 1.0f), projection);
//    output.color = color;

//    return output;
//}

#define CONCAT(a,b) a##b 
#define bReg(index) CONCAT(b,index)

cbuffer perView : register(b0)
{
    row_major matrix viewProjection;
}

cbuffer perFrame : register(b1)
{
    float4 iResolution;
    float iTime;
}

cbuffer instanceBuffer : register(b2)
{
    row_major matrix transform;
}

cbuffer meshData : register(b3)
{
    row_major matrix meshToModel;
}

struct VIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 tc : TC;
    float4 modelToWorld0 : TOWORLD0;
    float4 modelToWorld1 : TOWORLD1;
    float4 modelToWorld2 : TOWORLD2;
    float4 modelToWorld3 : TOWORLD3;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VOut main(VIn input)
{
    VOut output;
    

    matrix toWorld = matrix(input.modelToWorld0, input.modelToWorld1, input.modelToWorld2, input.modelToWorld3);
    //float4x4 toWorld = float4x4(float4(1, 0, 0, 0), float4(0, 1, 0, 0), float4(0, 0, 1, 0), float4(0, 0, 0, 1));
    
    output.position = mul(mul(mul(float4(input.pos, 1.0f), toWorld), meshToModel), viewProjection);
    output.color = output.position;

    return output;
}