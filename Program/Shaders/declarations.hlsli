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