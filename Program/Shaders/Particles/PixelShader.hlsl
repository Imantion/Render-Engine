#include "..\declarations.hlsli"
#include "..\opaqueShader\Lights.hlsli"

Texture2D g_smokeEMVA : register(t20);
Texture2D g_lightmapRLU : register(t21);
Texture2D g_lightmapDBF : register(t22);
Texture2D g_DepthTexture : register(t23);

#define THICKNESS 1.0f

float3 SixWayLightMap(float3 lightIrradiance, float3 lightDirection, float lightmapRLUDBF[6])
{
    static const float3 basis[6] =
    {
        float3(1, 0, 0), float3(-1, 0, 0), float3(0, 1, 0), float3(0, -1, 0), 
        float3(0, 0, -1), float3(0, 0, 1)
    };

    float3 result = 0.0f;
    for (int i = 0; i < 6; i++)
    {
        result += lightIrradiance * (lightmapRLUDBF[i] * max(dot(lightDirection, basis[i]),0.0f));
    }

    return result;
}

struct PSIn
{
    float4 position : SV_Position;
    float3 worldPos : WORLD;
    float4 rgba : RGBA;
    float time : TIME;
    float2 uvThis : UV_THIS;
    float2 uvNext : UV_NEXT;
};

void DepthClipping(inout float alpha, float3 UVnDepth)
{

    float sampledDepth = g_DepthTexture.SampleLevel(g_sampler, UVnDepth.xy, 0);
    sampledDepth = linearize_depth(sampledDepth, g_nearClip, g_farClip);
    UVnDepth.z = linearize_depth(UVnDepth.z, g_nearClip, g_farClip);
    
    float nearDiff = abs(sampledDepth - UVnDepth.z);
    float lerpValue = min(1.0, nearDiff / THICKNESS);
    float near = lerp(0.0, alpha, lerpValue);
    float clipLerp = min(1.0, UVnDepth.z / THICKNESS);
    float clip = lerp(0.0, alpha, clipLerp);
    
    alpha = min(near, clip);
}


float4 main(PSIn input) : SV_TARGET
{
    // ----------- sample motion-vectors -----------

    float2 mv0 = 2.0 * g_smokeEMVA.Sample(g_sampler, input.uvThis).gb - 1.0; // current frame motion-vector
    float2 mv1 = 2.0 * g_smokeEMVA.Sample(g_sampler, input.uvNext).gb - 1.0; // next frame motion-vector

// need to flip motion-vector Y specifically for the smoke textures:
    mv0.y = -mv0.y;
    mv1.y = -mv1.y;

// ----------- UV flowing along motion-vectors -----------

    static const float MV_SCALE = 0.0015; // adjusted for the smoke textures
    float time = input.time; // goes from 0.0 to 1.0 between two sequential frames

    float2 uv0 = input.uvThis; // texture sample uv for the current frame
    uv0 -= mv0 * MV_SCALE * time; // if MV points in some direction, then UV flows in opposite

    float2 uv1 = input.uvNext; // texture sample uv for the next frame
    uv1 -= mv1 * MV_SCALE * (time - 1.f); // if MV points in some direction, then UV flows in opposite

// ----------- sample textures -----------

    float2 emissionAlpha0 = g_smokeEMVA.Sample(g_sampler, uv0).ra;
    float2 emissionAlpha1 = g_smokeEMVA.Sample(g_sampler, uv1).ra;

// .x - right, .y - left, .z - up
    float3 lightmapRLU0 = g_lightmapRLU.Sample(g_sampler, uv0).rgb;
    float3 lightmapRLU1 = g_lightmapRLU.Sample(g_sampler, uv1).rgb;

// .x - down, .y - back, .z - front
    float3 lightmapDBF0 = g_lightmapDBF.Sample(g_sampler, uv0).rgb;
    float3 lightmapDBF1 = g_lightmapDBF.Sample(g_sampler, uv1).rgb;

// ----------- lerp values -----------

    float2 emissionAlpha = lerp(emissionAlpha0, emissionAlpha1, time);
    float3 lightmapRLU = lerp(lightmapRLU0, lightmapRLU1, time);
    float3 lightmapDBF = lerp(lightmapDBF0, lightmapDBF1, time);
    
    float lightmapRLUDBF[6] = { lightmapRLU.x, lightmapRLU.y, lightmapRLU.z, lightmapDBF.x, lightmapDBF.y, lightmapDBF.z };
    
    float3 finalRadiance = float3(0, 0, 0);
    
    for (int i = 0; i < plSize; i++)
    {
        float3 directionToLight = pointLights[i].position - input.worldPos;
        float solidAngle = SolidAngle(pointLights[i].radius, dot(directionToLight, directionToLight));
        finalRadiance += SixWayLightMap(pointLights[i].radiance * solidAngle, normalize(directionToLight), lightmapRLUDBF);
    }
    
    for (i = 0; i < slSize; i++)
    {
        float3 directionToLight = spotLights[i].position - input.worldPos;
        float solidAngle = SolidAngle(spotLights[i].radiusOfCone, dot(directionToLight, directionToLight));
        finalRadiance += SixWayLightMap(spotLights[i].radiance * SpotLightCuttOffFactor(spotLights[i], input.worldPos, g_cameraPosition) * solidAngle, normalize(directionToLight), lightmapRLUDBF);
    }
    
    for (i = 0; i < dlSize; i++)
    {
        finalRadiance += SixWayLightMap(directionalLights[i].radiance * directionalLights[i].solidAngle, -directionalLights[i].direction, lightmapRLUDBF);
    }
    
    float3 directionToLight = flashLight.position - input.worldPos;
    float solidAngle = SolidAngle(flashLight.radiusOfCone, dot(directionToLight, directionToLight));
    finalRadiance += SixWayLightMap(flashLight.radiance * SpotLightCuttOffFactor(flashLight, input.worldPos, g_cameraPosition), normalize(directionToLight), lightmapRLUDBF);
    finalRadiance += emissionAlpha.r;
    finalRadiance *= input.rgba.rgb;

    float4 output = float4(finalRadiance, input.rgba.a * emissionAlpha.g);
    float3 UVnDepth = float3((input.position.x - 0.5f) / g_viewportWidth, (input.position.y - 0.5f) / g_viewportHeight, input.position.z);
    DepthClipping(output.a, UVnDepth);
    
    return output;

}