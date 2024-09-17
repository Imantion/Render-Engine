#include "..\declarations.hlsli"

cbuffer meshData : register(b2)
{
    float4x4 meshToModel;
}

struct VIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 tc : TC;
    float4 modelToWorld[4] : TOWORLD;
    uint objectId : OBJECTID;
};


struct VOut
{
    float3 position : WORLDPOS;
    float3 normal : NORMAL;
};

VOut main(VIn input)
{
    VOut output;
    

    float4x4 toWorld = float4x4(input.modelToWorld[0], input.modelToWorld[1], input.modelToWorld[2], input.modelToWorld[3]);
    
    output.position = mul(mul(float4(input.pos, 1.0f), meshToModel), toWorld);
    
    
    float3 axisX = normalize(input.modelToWorld[0].xyz);
    float3 axisY = normalize(input.modelToWorld[1].xyz);
    float3 axisZ = normalize(input.modelToWorld[2].xyz);
    
    float3x3 meshToWorldNormalized = float3x3(normalize(meshToModel._11_12_13), normalize(meshToModel._21_22_23), normalize(meshToModel._31_32_33));
    float3 N = mul(input.normal, meshToWorldNormalized);
    N = normalize(N);
    output.normal = N;
    // local normal visualizationg
    //float3 axisX = normalize(meshToModel._11_21_31);
    //float3 axisY = normalize(meshToModel._12_22_32);
    //float3 axisZ = normalize(meshToModel._13_23_33);
    
    //float3 N = float4(input.normal, 1);
    
    
    //N = normalize(N);
    //float3 worldN = N.x * axisX + N.y * axisY + N.z * axisZ;
    
    //worldN = worldN * 0.5 + 0.5f;
    
    return output;
}