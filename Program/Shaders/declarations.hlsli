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

SamplerState g_pointWrap : register(s0); // No interpolation, Point Wrap
SamplerState g_linearWrap : register(s1); // Trilinear interpolation, Linear Wrap
SamplerState g_anisotropicWrap : register(s2); // Anisotropic filtering, Wrap
SamplerState g_sampler : register(s3); // Configurable filtering