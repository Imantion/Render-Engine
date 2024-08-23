#ifndef DECLARATIONS_M
#define DECLARATIONS_M

#pragma pack_matrix( row_major )

#define MAX_MIP 10

cbuffer perView : register(b0)
{
    matrix viewProjection;
    matrix inverseViewProjection;
    float4x4 g_viewMatrix;
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
    uint g_samplesAmount;
    float g_farClip;
    float g_nearClip;
    float g_texelWidth;
    float g_texelHeight;
}

inline float linearize_depth(float d, float zNear, float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

SamplerState g_pointWrap : register(s0); // No interpolation, Point Wrap
SamplerState g_linearWrap : register(s1); // Trilinear interpolation, Linear Wrap
SamplerState g_anisotropicWrap : register(s2); // Anisotropic filtering, Wrap
SamplerState g_sampler : register(s3); // Configurable filtering

float2 nonZeroSign(float2 v)
{
	return float2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
}

float2 packOctahedron(float3 v)
{
	float2 p = v.xy / (abs(v.x) + abs(v.y) + abs(v.z));
	return v.z <= 0.0 ? (float2(1.0f,1.0f) - abs(p.yx)) * nonZeroSign(p) : p;
}

float3 unpackOctahedron(float2 oct)
{
    float3 v = float3(oct, 1.0 - abs(oct.x) - abs(oct.y));
    if (v.z < 0)
        v.xy = (float2(1.0f ,1.0f) - abs(v.yx)) * nonZeroSign(v.xy);
    return normalize(v);
}


#endif