#include "../declarations.hlsli"

struct PSIn
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3 normal : NORMAL;
    nointerpolation float3 emission : EMISSION;
    nointerpolation float3 particlePos : PARTICLEPOS;
};

Texture2D<float4> albedo : register(t3);
Texture2D<float4> normals : register(t4);
Texture2D<float> depth : register(t5);

float4 main(PSIn input) : SV_TARGET
{
	float depthValue = depth.Load(int3(input.pos.xy, 0));
    float4 octaNormal = normals.Load(int3(input.pos.xy, 0)).xyzw;
    float3 normal = unpackOctahedron(octaNormal.xy);
    float3 geometryNormal = unpackOctahedron(octaNormal.zw);
    float3 clipSpace = float3(input.pos.x - 0.5f, input.pos.y - 0.5f, depthValue);
    clipSpace.x *= g_texelWidth;
    clipSpace.y *= g_texelHeight;
    clipSpace.y = 1 - clipSpace.y;
    clipSpace.xy = (clipSpace.xy - 0.5f) * 2.0f;
    float4 worldSpace = mul(float4(clipSpace, 1.0f), inverseViewProjection);
    float3 objectPos = worldSpace.xyz / worldSpace.w;
    
    float3 lightDir = (input.particlePos + geometryNormal * 0.01f) -objectPos;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);
    
        // Define your parameters
    float minDistance = 0.0f;
    float maxDistance = 0.1f;

    // Clamp the distance between min and max
    distance = saturate((distance - minDistance) / (maxDistance - minDistance));

    // Compute attenuation based on the distance
    float attenuation = 1.0f - distance;

   
    float3 albedoValue = albedo.Load(int3(input.pos.xy, 0)).xyz;
    float3 irradiance = albedoValue * input.emission * (max(0.0001f, dot(normal, lightDir)) * 15 * attenuation);

    return float4(irradiance, 0.0f);
}