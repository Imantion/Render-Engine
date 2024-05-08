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

cbuffer instanceBuffer : register(b3)
{
    row_major matrix transform;
}

cbuffer meshData : register(b4)
{
    row_major matrix meshToModel;
}