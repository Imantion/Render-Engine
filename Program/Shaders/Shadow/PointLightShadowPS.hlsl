cbuffer linearDepthTransformData : register(b10)
{
    float3 lightPosition;
    float farPlane;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : WORLDPOS;
    uint renderTargetArrayIndex : SV_RenderTargetArrayIndex;
};

float main(PSInput input) : SV_DEPTH
{
    float distance = length(lightPosition - input.worldPos);
    return 1.0f - distance / farPlane;
}