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
static const float g_MIN_F0 = 0.01;



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
    float3 directionToLight = spotLight.position - position;
    directionToLight = normalize(directionToLight);
        
    float cosAngle = dot(directionToLight, -spotLight.direction);
    if (cosAngle < spotLight.cutoffAngle)
        return 0.001f;
    else
        return 1 - (1 - cosAngle) / (1 - spotLight.cutoffAngle);
}

// May return direction pointing beneath surface horizon (dot(N, dir) < 0), use clampDirToHorizon to fix it.
// sphereCos is cosine of the light source angular halfsize (2D angle, not solid angle).
// sphereRelPos is position of a sphere relative to surface:
// 'sphereDir == normalize(sphereRelPos)' and 'sphereDir * sphereDist == sphereRelPos'
float3 approximateClosestSphereDir(out bool intersects, float3 reflectionDir, float sphereCos,
    float3 sphereRelPos, float3 sphereDir, float sphereDist, float sphereRadius)
{
    float RoS = dot(reflectionDir, sphereDir);

    intersects = RoS >= sphereCos;
    if (intersects)
        return reflectionDir;
    if (RoS < 0.0)
        return sphereDir;

    float3 closestPointDir = normalize(reflectionDir * sphereDist * RoS - sphereRelPos);
    return normalize(sphereRelPos + sphereRadius * closestPointDir);
}

// Input dir and NoD is N and NoL in a case of lighting computation 
void clampDirToHorizon(inout float3 dir, inout float NoD, float3 normal, float minNoD)
{
    if (NoD < minNoD)
    {
        dir = normalize(dir + (minNoD - NoD) * normal);
        NoD = minNoD;
    }
}

// [ de Carpentier 2017, "Decima Engine: Advances in Lighting and AA" ]
// sphereSin and sphereCos are sin and cos of the light source angular halfsize (2D angle, not solid angle).
void SphereMaxNoH(float NoV, inout float NoL, inout float VoL, float sphereSin, float sphereCos, bool bNewtonIteration, out float NoH, out float VoH)
{
    float RoL = 2 * NoL * NoV - VoL;
    if (RoL >= sphereCos)
    {
        NoH = 1;
        VoH = abs(NoV);
    }
    else
    {
        float rInvLengthT = sphereSin * rsqrt(1 - RoL * RoL);
        float NoTr = rInvLengthT * (NoV - RoL * NoL);
        float VoTr = rInvLengthT * (2 * NoV * NoV - 1 - RoL * VoL);

        if (bNewtonIteration && sphereSin != 0)
        {
			// dot( cross(N,L), V )
            float NxLoV = sqrt(saturate(1 - pow(NoL, 2) - pow(NoV, 2) - pow(VoL, 2) + 2 * NoL * NoV * VoL));

            float NoBr = rInvLengthT * NxLoV;
            float VoBr = rInvLengthT * NxLoV * 2 * NoV;

            float NoLVTr = NoL * sphereCos + NoV + NoTr;
            float VoLVTr = VoL * sphereCos + 1 + VoTr;

            float p = NoBr * VoLVTr;
            float q = NoLVTr * VoLVTr;
            float s = VoBr * NoLVTr;

            float xNum = q * (-0.5 * p + 0.25 * VoBr * NoLVTr);
            float xDenom = p * p + s * (s - 2 * p) + NoLVTr * ((NoL * sphereCos + NoV) * pow(VoLVTr, 2) + q * (-0.5 * (VoLVTr + VoL * sphereCos) - 0.5));
            float TwoX1 = 2 * xNum / (pow(xDenom, 2) + pow(xNum, 2));
            float SinTheta = TwoX1 * xDenom;
            float CosTheta = 1.0 - TwoX1 * xNum;
            NoTr = CosTheta * NoTr + SinTheta * NoBr;
            VoTr = CosTheta * VoTr + SinTheta * VoBr;
        }

        NoL = NoL * sphereCos + NoTr;
        VoL = VoL * sphereCos + VoTr;

        float InvLenH = rsqrt(2 + 2 * VoL);
        NoH = saturate((NoL + NoV) * InvLenH);
        VoH = saturate(InvLenH + InvLenH * VoL);
    }
}

float horizonFalloffFactor(float3 normal, float3 planePoint, float3 lightPosition, float lightRadius)
{
    float distance = normal.x * (lightPosition.x - planePoint.x) + normal.y * (lightPosition.y - planePoint.y) + normal.z * (lightPosition.z - planePoint.z);
    float sum = max(distance + lightRadius, 0.0f);
    return min(1, sum / 2.0f * lightRadius);
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
    float denominator = max((sqrt(1.0f + roughness4 * (1 - NoV) / max(NoV, 0.0001f)) * sqrt(1.0f + roughness4 * (1 - NoL) / max(NoL, 0.0001f))), 0.0001f);
    return 2.0f / denominator;
}

float SolidAngle(float radius, float distanceSquared)
{
    return 2 * PI * (1 - sqrt(1 - min(radius * radius / distanceSquared, 1.0f)));
}


float3 PBRLight(float3 irradiance, float solidAngle, float3 l ,float3 albedo, float metalness, float roughness, float3 n, float3 v, bool specular = true, bool diffuse = true)
{
    float3 h = normalize(v + l);
    float rSquared = roughness * roughness;
    float NoH = max(dot(n, h), 0.001f);
    float NoV = max(dot(n, v), 0.001f);
    float NoL = max(dot(n, l), 0.001f);
    float HoL = max(dot(h, l), 0.001f);
    float3 F0 = lerp(g_MIN_F0, albedo, metalness);

    
    float3 f_spec = 0.0f;
    if (specular)
        f_spec = min(D_GGX(rSquared, NoH) * solidAngle / (4 * NoV), 1.0f) * G_Smith(rSquared, NoV, NoL) * fresnel(F0, HoL);
    
    float3 f_diff = 0.0f;
    if(diffuse)
        f_diff = (1 - metalness) / PI * solidAngle * NoL * albedo * (1 - fresnel(F0, NoL));
    
    return irradiance * (f_diff + f_spec);
}

float3 PBRLight(PointLight lightSource, float3 worldPosition, float3 albedo, float metalness, float roughness, float3 macroNormal, float3 microNormal, 
               float3 v, bool specular = true, bool diffuse = true)
{
    float3 relPosition = lightSource.position - worldPosition;
    float3 lightDir = normalize(relPosition);
    float distance = length(relPosition);
    float solidAngle = SolidAngle(lightSource.radius, distance * distance);
    float cosAnglular = sqrt(1.0f / (1.0f + pow((lightSource.radius / distance), 2)));
    float sinAngular = sqrt(1 - cosAnglular * cosAnglular);
    bool intersects;
    float3 l = approximateClosestSphereDir(intersects, reflect(lightDir, microNormal), cosAnglular, relPosition, lightDir, distance, lightSource.radius);
    float NoL = dot(microNormal, l);
    clampDirToHorizon(l, NoL, microNormal, 0.001f);
    
   

    float3 h = normalize(v + l);
    float rSquared = roughness * roughness;
    float NoH = max(dot(microNormal, h), 0.001f);
    float NoV = max(dot(microNormal, v), 0.001f);
    float HoL = max(dot(h, l), 0.001f);
    float VoL = max(dot(v, l), 0.001f);
    float3 F0 = lerp(0.4f, albedo, metalness);

    SphereMaxNoH(NoV, NoL, VoL, sinAngular, cosAnglular, true, NoH, NoV);
    
    float3 f_spec = 0.0f;
    if(specular)
        f_spec = min(D_GGX(rSquared, NoH) * solidAngle / (4 * NoV), 1.0f) * G_Smith(rSquared, NoV, NoL) * fresnel(F0, HoL);
    
    float3 f_diff = 0.0f;
    if (diffuse)
        f_diff = (1 - metalness) / PI * solidAngle * NoL * albedo * (1 - fresnel(F0, NoL));
    

    return lightSource.color * (f_diff + f_spec) * horizonFalloffFactor(microNormal, worldPosition, lightSource.position, lightSource.radius) *
    horizonFalloffFactor(macroNormal, worldPosition, lightSource.position, lightSource.radius);
}

float3 FlashLight(SpotLight flashLight,float3 albedo, float metalness, float roughness, float3 normal, float3 position, float3 cameraPosition, bool specular, bool diffuse)
{
    float3 directionToLight = flashLight.position - position;
    float3 view = normalize(cameraPosition - position);
    float solidAngle = SolidAngle(flashLight.radiusOfCone, dot(directionToLight, directionToLight));
    float3 finalColor = SpotLightCuttOffFactor(flashLight, position, cameraPosition) * PBRLight(flashLight.color, solidAngle, normalize(directionToLight), albedo, 
                                                                                                metalness, roughness, normal, view, specular, diffuse);
    float4 proj = mul(float4(position, 1.0f), lightViewProjection);
    float2 uv = (proj.xy) / proj.w;
    float3 mask = flashlighTexture.Sample(samplerstateFlash, uv * 0.5 + 0.5).rgb;
    
    return finalColor * mask;
}




 
