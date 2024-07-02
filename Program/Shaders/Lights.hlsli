#if MAX_DIRECTIONAL_LIGHTS
static const int MAX_DL = MAX_DIRECTIONAL_LIGHTS;
#else
static const int MAX_DL = 1;
#endif

#if MAX_POINT_LIGHTS
static const int MAX_PL = MAX_POINT_LIGHTS;
#else
static const int MAX_PL = 5;
#endif

#if MAX_SPOT_LIGHTS
static const int MAX_SL = MAX_SPOT_LIGHTS;
#else
static const int MAX_SL = 5;
#endif

#define PI 3.141

static const float3 ambient = float3(0.005f, 0.005f, 0.005f);
Texture2D flashlighTexture : register(t1);
SamplerState samplerstateFlash : register(s0);

struct DirectionalLight
{
    float3 color;
    float solidAngle;
    float3 direction;
    float padding; // Padding to align structure size
};

// PointLight structure
struct PointLight
{
    float3 color;
    float radius;
    float3 position;
    int bindedObjectId; // -1 means no object is bound
};

// SpotLight structure
struct SpotLight
{
    float3 color;
    float radiusOfCone;
    float3 direction;
    float cutoffAngle; // in radians
    float3 position;
    int bindedObjectId; // -1 means no object is bound
};

cbuffer LightData : register(b3)
{
    DirectionalLight directionalLights[MAX_DL];
    PointLight pointLights[MAX_PL];
    SpotLight spotLights[MAX_SL];
    SpotLight flashLight;
    float4x4 lightViewProjection;
    uint dlSize;
    uint plSize;
    uint slSize;
}

float SpotLightCuttOffFactor(SpotLight spotLight, float3 position, float3 cameraPosition)
{
    float3 directionToLight = position - spotLight.position;
    directionToLight = normalize(directionToLight);
        
    float cosAngle = dot(directionToLight, spotLight.direction);
    if (cosAngle < spotLight.cutoffAngle)
        return 0.001f;
    else
        return 1 - (1 - cosAngle) / (1 - spotLight.cutoffAngle);
}
float3 fresnel(float3 F0, float angle)
{
    return F0 + (1.0f - F0) * pow(1 - angle, 5);
}

float D_GGX(float roughness4, float NoH)
{
    float denominator = NoH * NoH * (roughness4 - 1) + 1;
    denominator = PI * denominator * denominator;
    denominator = max(denominator, 0.0001f);
    
    return roughness4 / denominator;
}

float G_Smith(float roughness4, float NoV, float NoL)
{
    float denominator = max((sqrt(1.0f + roughness4 * (1 - NoV) / NoV) * sqrt(1.0f + roughness4 * (1 - NoL) / NoL)), 0.0001f);
    return 2.0f / denominator;
}

float radianceFromIrradiance(float irradiance, float radius, float distanceSquared)
{
    return irradiance / (1 - sqrt(1 - min(radius * radius / distanceSquared, 1.0f)));
}


float3 BRDF(float3 albedo, float metalness, float roughness, float3 n, float3 v, float3 l)
{
    float3 h = normalize(v + l);
    float rSquared = roughness * roughness;
    float NoH = max(dot(n, h), 0.001f);
    float NoV = max(dot(n, v),0.001f);
    float NoL = max(dot(n, l), 0.001f);
    float HoL = max(dot(h, l), 0.001f);
    float3 F0 = lerp(0.4f, albedo, metalness);

    
    float3 f_spec = D_GGX(rSquared, NoH) * G_Smith(rSquared, NoV, NoL) / (4 * NoL * NoV) * fresnel(F0, HoL);
    float3 f_diff = (1 - metalness) / PI * albedo * (1 - fresnel(F0, NoL));

    return f_spec + f_diff;
}


float SolidAngle(float radius, float distanceSquared)
{
    return 2 * PI * (1 - sqrt(1 - min(radius * radius / distanceSquared, 1.0f)));
}


float3 PBRLight(float3 irradiance, float solidAngle, float3 l ,float3 albedo, float metalness, float roughness, float3 n, float3 v)
{
    return irradiance * BRDF(albedo, metalness, roughness, n, v, l) * (max(dot(n, l), 0.001f) * solidAngle);
}

float3 FlashLight(SpotLight flashLight,float3 albedo, float metalness, float roughness, float3 normal, float3 position, float3 cameraPosition)
{
    float3 directionToLight = flashLight.position - position;
    float3 view = cameraPosition - position;
    float3 finalColor = SpotLightCuttOffFactor(flashLight, position, cameraPosition) * SolidAngle(flashLight.radiusOfCone, dot(directionToLight, directionToLight)) * 
                        BRDF(albedo, metalness, roughness, normal, view, directionToLight);
    float4 proj = mul(float4(position, 1.0f), lightViewProjection);
    float2 uv = (proj.xy) / proj.w;
    float3 mask = flashlighTexture.Sample(samplerstateFlash, uv * 0.5 + 0.5).rgb;
    
    return finalColor * mask;
}




 
