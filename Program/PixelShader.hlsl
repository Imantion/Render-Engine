#define FIRST


#ifdef FIRST

float4 main(float4 fragCoord : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return color;
}

#else

#define pi 3.14159265359

cbuffer ConstantBuffer : register(b0)
{
    float4 iResolution;
    float iTime;
}


float saw(float x)
{
    return abs(frac(x) - 0.5) * 2.0;
}


float dw(float2 p, float2 c, float t)
{
    return sin(length(p - c) - t);
}


float dw1(float2 uv)
{
    float v = 0.0;
    float t = iTime * 2.0;
    v += dw(uv, float2(sin(t * 0.07) * 30.0, cos(t * 0.04) * 20.0), t * 1.3);
    v += dw(uv, float2(cos(t * 0.13) * 30.0, sin(t * 0.14) * 20.0), t * 1.6 + 1.0);
    v += dw(uv, float2(18, -15), t * 0.7 + 2.0);
    v += dw(uv, float2(-18, 15), t * 1.1 - 1.0);
    return v / 4.0;
}


float fun(float x, float y)
{
    return dw1(float2(x - 0.5, y - 0.5) * 80.0);
}


float3 duv(float2 uv)
{
    float x = uv.x;
    float y = uv.y;
    float v = fun(x, y);
    float d = 1.0 / 400.0;
    float dx = (v - fun(x + d, y)) / d;
    float dy = (v - fun(x, y + d)) / d;
    float a = atan2(dy, dx) / pi / 2.0;
    return float3(v, 0, (v * 4.0 + a));
}


float4 main(float4 fragCoord : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    float2 uv = fragCoord.xy / iResolution.x;
    float3 h = duv(uv);
    float sp = saw(h.z + iTime * 1.3);
    sp = clamp((sp - 0.25) * 2.0, 0.5, 1.0);
    return float4((h.x + 0.5) * sp, (0.3 + saw(h.x + 0.5) * 0.6) * sp, (0.6 - h.x) * sp, 1.0);
}

#endif