#include "../declarations.hlsli"

cbuffer MeshData : register(b3)
{
    float4x4 meshToModel;
};

struct VOut
{
    float4 position : SV_POSITION;
    float3 vPosition : WORLDPOS;
    float3 normal : NORMAL;
};

float4 main(VOut input) : SV_TARGET
{
    //float3 axisX = normalize(input.modelToWorld[0].xyz);
    //float3 axisY = normalize(input.modelToWorld[1].xyz);
    //float3 axisZ = normalize(input.modelToWorld[2].xyz);
    
    //float3x3 meshToWorldNormalized = float3x3(normalize(meshToModel._11_12_13), normalize(meshToModel._21_22_23), normalize(meshToModel._31_32_33));
    //float3 N = mul(input.normal, meshToWorldNormalized);
    //N = normalize(N);
    
    //float3 worldN = N.x * axisX + N.y * axisY + N.z * axisZ;
    float3 worldN = input.normal * 0.5 + 0.5f;
    
    return float4(worldN, 1.0f);
}

