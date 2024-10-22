#include "opaque.hlsli"

struct PSInput
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3x3 tbn : TBN;
    float2 tc : TC;
    nointerpolation int isSelected : ISSELECTED;
    nointerpolation int shouldOverWriteMaterial : SHOULDOVERWRITE;
    nointerpolation float roughness : ROUGHNESS;
    nointerpolation float metalness : METALNESS;
    nointerpolation uint objectId : OBJECTID;
};

struct PSOutput
{
    float4 albedo : SV_Target0;
    float4 roughMetal : SV_Target1;
    float4 normals : SV_Target2;
    float4 emission : SV_Target3;
    uint objectId : SV_Target4;
    
};

PSOutput main(PSInput input)
{
    PSOutput output;
    
    output.albedo = CalculateAlbedo4(input.tc);
    if (output.albedo.a < 0.05)
        discard;
    
    CalculateMaterialProperties(input.tc, output.roughMetal.y, output.roughMetal.x);
    
    if (input.isSelected)
    {
        if (input.shouldOverWriteMaterial)
        {
            output.roughMetal.x = input.roughness;
            output.roughMetal.y = input.metalness;
        }
        else
        {
            output.roughMetal.x = saturate(output.roughMetal.x * input.roughness);
            output.roughMetal.y = saturate(output.roughMetal.y * input.metalness);
        }
    }
    
    float3 microNormal = CalculateNormal(input.tc, input.tbn);

    output.normals.xy = packOctahedron(microNormal);
    output.normals.zw = packOctahedron(input.tbn._31_32_33);
    
    output.emission = float4(0, 0, 0, 1);
    output.objectId = input.objectId;
    
    output.roughMetal.zw = float2(0, 1);
    
    return output;
}