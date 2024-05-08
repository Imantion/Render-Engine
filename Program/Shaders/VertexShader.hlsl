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
    float4 modelToWorld[4] : TOWORLD;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VOut main(VIn input)
{
    VOut output;
    

    matrix toWorld = matrix(input.modelToWorld[0], input.modelToWorld[1], input.modelToWorld[2], input.modelToWorld[3]);
    
    output.position = mul(mul(mul(float4(input.pos, 1.0f), meshToModel), toWorld), viewProjection);
    
    float3 axisX = normalize(toWorld[0].xyz);
    float3 axisY = normalize(toWorld[1].xyz);
    float3 axisZ = normalize(toWorld[2].xyz);
  
    float3 worldN = input.normal.x * axisX + input.normal.y * axisY + input.normal.z * axisZ;
    
    worldN = worldN * 0.5 + 0.5f;
    
    output.color = float4(worldN, 1);
    return output;
}