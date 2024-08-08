#ifndef DECLARATIONS_M
#define DECLARATIONS_M

#pragma pack_matrix( row_major )

#define MAX_MIP 10

cbuffer perView : register(b0)
{
    matrix viewProjection;
    float3 bottomLeft;
    float3 topVector;
    float3 rightVector;
    float3 g_cameraPosition;
}

cbuffer perFrame : register(b1)
{
    float4 iResolution;
    float g_time;
    int diffuseState;
    int specularState;
    int IBLState;
    int LTCState;
    float g_shadowResolution;
    float g_PointLightFarPlane;
}

SamplerState g_pointWrap : register(s0); // No interpolation, Point Wrap
SamplerState g_linearWrap : register(s1); // Trilinear interpolation, Linear Wrap
SamplerState g_anisotropicWrap : register(s2); // Anisotropic filtering, Wrap
SamplerState g_sampler : register(s3); // Configurable filtering

#endif