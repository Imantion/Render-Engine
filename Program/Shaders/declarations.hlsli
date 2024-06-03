#pragma pack_matrix( row_major )

cbuffer perView : register(b0)
{
    matrix viewProjection;
    matrix projection;
    matrix view;
    float3 g_cameraPosition;
}

cbuffer perFrame : register(b1)
{
    float4 iResolution;
    float g_time;
}