struct PSInput
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    uint renderTargetArrayIndex : SV_RenderTargetArrayIndex;
};

float4 main(PSInput input) : SV_TARGET
{
    return input.color;
}
