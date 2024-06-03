TextureCube text : register(t0);
SamplerState samp : register(s0);

struct psOutput
{
    float4 pos : SV_POSITION;
    float3 direction : TEXTCOORD0;
};


float4 main(psOutput direction) : SV_TARGET
{
    return text.Sample(samp, direction.direction);
}