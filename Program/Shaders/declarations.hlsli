cbuffer perView : register(b0)
{
    row_major matrix viewProjection;
    float3 g_cameraPosition;
}

cbuffer perFrame : register(b1)
{
    float4 iResolution;
    float g_time;
}