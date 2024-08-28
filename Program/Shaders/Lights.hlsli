#if MAX_DIRECTIONAL_LIGHTS
static const int MAX_DL = MAX_DIRECTIONAL_LIGHTS;
#else
static const int MAX_DL = 1;
#endif

#if MAX_POINT_LIGHTS
static const int MAX_PL = MAX_POINT_LIGHTS;
#else
static const int MAX_PL = 10;
#endif

#if MAX_SPOT_LIGHTS
static const int MAX_SL = MAX_SPOT_LIGHTS;
#else
static const int MAX_SL = 10;
#endif

#if MAX_AREA_LIGHTS
static const int MAX_AL = MAX_AREA_LIGHTS;
#else
static const int MAX_AL = 1;
#endif

#if MAX_AREA_LIGHTS_VERTICES
static const int MAX_AREA_VERT = MAX_AREA_LIGHTS_VERTICES;
#else
static const int MAX_AREA_VERT = 4;
#endif

#if MAX_AREA_LIGHTS_INDICES
static const int MAX_AREA_IND = MAX_AREA_LIGHTS_INDICES;
#else
static const int MAX_AREA_IND = 4;
#endif

#define PI 3.141
#define SHADOW_DEPTH_OFFSET 0.02f

static const float g_MIN_F0 = 0.01;

Texture2D flashlighTexture : register(t1);
SamplerState samplerstateFlash : register(s4);

Texture2D albed : register(t2);
Texture2D rough : register(t3);
Texture2D metal : register(t4);
Texture2D normalTexture : register(t5);

TextureCube diffuseIBL : register(t6);
TextureCube specIrrIBL : register(t7);
Texture2D reflectanceIBL : register(t8);
Texture2D LTCmat : register(t9);
Texture2D LTCamp : register(t10);

TextureCubeArray pointLightsShadowMap : register(t11);
Texture2DArray spotLightsShadowMap : register(t12);
Texture2DArray directionalLightsShadowMap : register(t13);

SamplerComparisonState compr : register(s5);

struct DirectionalLight
{
    float3 radiance;
    float solidAngle;
    float3 direction;
    float padding; // Padding to align structure size
};

// PointLight structure
struct PointLight
{
    float3 radiance;
    float radius;
    float3 position;
    int bindedObjectId; // -1 means no object is bound
};

// SpotLight structure
struct SpotLight
{
    float3 radiance;
    float radiusOfCone;
    float3 direction;
    float cutoffAngle; // in radians
    float3 position;
    int bindedObjectId; // -1 means no object is bound
};

struct edge
{
    uint v1, v2;
    uint padding1, padding2;
};

struct AreaLight
{
    float3 radiance;
    uint verticesAmount;
    float3 vertices[MAX_AREA_VERT];
    edge boundedIndices[MAX_AREA_IND];
    uint indicesAmount;
    float intensity;
};

cbuffer LightData : register(b3)
{
    AreaLight areaLights[MAX_AL];
    DirectionalLight directionalLights[MAX_DL];
    PointLight pointLights[MAX_PL];
    SpotLight spotLights[MAX_SL];
    SpotLight flashLight;
    float4x4 lightViewProjection;
    uint dlSize;
    uint plSize;
    uint slSize;
    uint alSize;
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
    return min(1, sum / (2.0f * lightRadius));
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
    NoV *= NoV;
    NoL *= NoL;
    return 2.0 / (sqrt(1 + roughness4 * (1 - NoV) / NoV) + sqrt(1 + roughness4 * (1 - NoL) / NoL));
}

float SolidAngle(float radius, float distanceSquared)
{
    return 2 * PI * (1 - sqrt(1 - min(radius * radius / distanceSquared, 1.0f)));
}


float3 PBRLight(float3 irradiance, float solidAngle, float3 l, float3 albedo, float metalness, float roughness, float3 n, float3 v, bool specular = true, bool diffuse = true)
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
    if (diffuse)
        f_diff = (1 - metalness) / PI * solidAngle * NoL * albedo * (1 - fresnel(F0, NoL));
    
    return irradiance * (f_diff + f_spec);
}

float3 PBRLight(PointLight lightSource, float3 worldPosition, float3 albedo, float metalness, float roughness, float3 macroNormal, float3 microNormal,
               float3 v, bool specular = true, bool diffuse = true)
{
    float3 relPosition = lightSource.position - worldPosition;
    if(dot(relPosition,macroNormal) < -lightSource.radius)
        return 0.0f;
    
    float3 lightDir = normalize(relPosition);
    float distance = length(relPosition);
    distance = max(distance, lightSource.radius);
    
    float solidAngle = SolidAngle(lightSource.radius, distance * distance);
    float sinAngular = lightSource.radius / distance;
    float cosAnglular = sqrt(1.0f - sinAngular * sinAngular);
    
    bool intersects;
    float3 l = approximateClosestSphereDir(intersects, reflect(-v, microNormal), cosAnglular, relPosition, lightDir, distance, lightSource.radius);
   

    float map_fading = 0.05f;
    float NoL = max(dot(microNormal, lightDir), map_fading * sinAngular);
    float closestSphereNoL = dot(microNormal, l);
    clampDirToHorizon(l, closestSphereNoL, microNormal, 0.001f);
   

    float3 h = normalize(v + l);
    float NoH = max(dot(microNormal, h), 0.001);
    float rSquared = roughness * roughness;
    float NoV = max(dot(microNormal, v), 0.001f);
    float HoL = max(dot(h, l), 0.001f);
    float VoL = dot(v, l);
    float3 F0 = lerp(g_MIN_F0, albedo, metalness);
    
    float3 f_spec = 0.0f;
    if (specular)
        f_spec = min(D_GGX(rSquared, NoH) * solidAngle / (4 * NoV), 1.0f) * G_Smith(rSquared, NoV, closestSphereNoL) * fresnel(F0, HoL);
    
    float3 f_diff = 0.0f;
    if (diffuse)
        f_diff = (1 - metalness) / PI * solidAngle * NoL * albedo * (1 - fresnel(F0, NoL));
    

    return lightSource.radiance * (f_diff + f_spec);
}

float3 FlashLight(SpotLight flashLight, float3 albedo, float metalness, float roughness, float3 normal, float3 position, float3 cameraPosition, bool specular, bool diffuse)
{
    float3 directionToLight = flashLight.position - position;
    float3 view = normalize(cameraPosition - position);
    float solidAngle = SolidAngle(flashLight.radiusOfCone, dot(directionToLight, directionToLight));
    float3 finalColor = SpotLightCuttOffFactor(flashLight, position, cameraPosition) * PBRLight(flashLight.radiance, solidAngle, normalize(directionToLight), albedo,
                                                                                                metalness, roughness, normal, view, specular, diffuse);
    float4 proj = mul(float4(position, 1.0f), lightViewProjection);
    float2 uv = (proj.xy) / proj.w;
    float3 mask = flashlighTexture.Sample(samplerstateFlash, uv * 0.5 + 0.5).rgb;
    
    return finalColor * mask;
}

static const float LUT_SIZE = 64.0; // ltc_texture size
static const float LUT_SCALE = (64.0 - 1.0) / 64.0;
static const float LUT_BIAS = 0.5 / 64.0;

float3 IntegrateEdge(float3 v1, float3 v2, float3 N)
{
    float x = dot(v1, v2);
    float y = abs(x);
    float a = 0.8543985 + (0.4965155 + 0.0145206 * y) * y;
    float b = 3.4175940 + (4.1616724 + y) * y;
    float v = a / b;
    float theta_sintheta = (x > 0.0) ? v : 0.5 * rsqrt(max(1.0 - x * x, 1e-7)) - v;
    return dot(cross(v1, v2) * theta_sintheta, N);
}

float3 LTC_Evaluate(float3 N, float3 V, float3 P, float3x3 Minv, AreaLight areaLight, bool twoSided)
{
    // Construct orthonormal basis around N
    float3 T1 = normalize(V - N * dot(V, N));
    float3 T2 = cross(N, T1);

    // Rotate area light in (T1, T2, N) basis
    Minv = mul(Minv, float3x3(T1, T2, N));

    // Polygon (allocate 4 vertices for clipping)
    float3 L[MAX_AREA_VERT];

    for (int i = 0; i < areaLight.verticesAmount; i++)
    {
         // Transform polygon from LTC back to origin Do (cosine weighted)
        L[i] = normalize(mul(Minv, (areaLight.vertices[i] - P)));
    }

    // Use tabulated horizon-clipped sphere
    // Check if the shading point is behind the light
    float3 dir = areaLight.vertices[0] - P; // LTC space
    float3 lightNormal = cross(areaLight.vertices[1] - areaLight.vertices[0], areaLight.vertices[2] - areaLight.vertices[1]);
    bool behind = (dot(dir, lightNormal) < 0.0);



    // Integrate
    float3 vsum = float3(0.0, 0.0, 0.0);
    
    for (i = 0; i < areaLight.indicesAmount; i++)
    {
        vsum += IntegrateEdge(L[areaLight.boundedIndices[i].v1], L[areaLight.boundedIndices[i].v2], N);

    }

    // Form factor of the polygon in direction vsum
    float len = length(vsum);

    float z = vsum.z / len;
    if (behind)
        z = -z;

    float sum = len;
    if (!behind && !twoSided || dot(N, dir) < 0)
        sum = 0.0;
    

    // Outgoing radiance (solid angle) for the entire polygon
    float3 Lo_i = float3(sum, sum, sum);
    return Lo_i;
}

uint selectCubeFace(float3 unitDir)
{
    float maxVal = max(abs(unitDir.x), max(abs(unitDir.y), abs(unitDir.z)));
    uint maxIndex = abs(unitDir.x) == maxVal ? 0 : (abs(unitDir.y) == maxVal ? 2 : 4);
    return maxIndex + (asuint(unitDir[maxIndex / 2]) >> 31); // same as:
    // return maxIndex + (unitDir[maxIndex / 2] < 0.f ? 1u : 0u);
}

float3 offset(float shadowTexelSize, float3 normal, float3 lightDir)
{
    float denominator = sqrt(2) * 0.5f;
    return shadowTexelSize * denominator * (normal - lightDir * (0.9f * dot(normal, lightDir)));
}

float3 LTC(AreaLight areaLight,float3 worldPos, float3 normal, float3 view, float3 albedo, float roughness, float metalness)
{
    float dotNV = clamp(dot(normal, view), 0.0f, 1.0f);

    // use roughness and sqrt(1-cos_theta) to sample M_texture
    float2 uv = float2(roughness, sqrt(1.0f - dotNV));
    uv = uv * LUT_SCALE + LUT_BIAS;

    float4 t1 = LTCmat.Sample(samplerstateFlash, uv);
    float t2 = LTCamp.Sample(samplerstateFlash, uv);
    
    float3x3 Minv = float3x3(
    float3(t1.x, 0, t1.y),
    float3(0, 1, 0),
    float3(t1.z, 0, t1.w)
    );
    
    float3x3 Identity =
    {
        { 1, 0, 0, },
        { 0, 1, 0, },
        { 0, 0, 1 },
    };
    
    float3 d = LTC_Evaluate(normal, view, worldPos, Identity, areaLight, true);
    float3 s = LTC_Evaluate(normal, view, worldPos, Minv, areaLight, true);
    return areaLight.radiance * (d * albedo * (1 - metalness) + s) * (t2.r * areaLight.intensity);
}


float PCF(Texture2DArray textureArray, SamplerComparisonState compSampler, int index, float3 projectedCoord, float texelSize)
{
    static const float2 offsets[9] =
    {
        float2(-1.0, -1.0), float2(0.0, -1.0), float2(1.0, -1.0),
    float2(-1.0, 0.0), float2(0.0, 0.0), float2(1.0, 0.0),
    float2(-1.0, 1.0), float2(0.0, 1.0), float2(1.0, 1.0)
    };
    
    float shadowValue = 0.0f;
    for (int i = 0; i < 9; i++)
    {
        shadowValue += textureArray.SampleCmpLevelZero(compSampler, float3(projectedCoord.xy + offsets[i] * texelSize * 0.5f, index), projectedCoord.z);
    }
    shadowValue = shadowValue / 9.0f;
    return smoothstep(0.33, 1.0f, shadowValue);
}

float3 worldToUV(float3 lightPos, float3 worldPos, float3 normal, float4x4 viewProjectionMatrix)
{
    float3 directionToLight = lightPos - worldPos;
    float4 projected = mul(float4(worldPos + offset(1.0f / g_shadowResolution, normal, directionToLight), 1.0f), viewProjectionMatrix);
    float3 homogeneus = projected.xyz / projected.w;
    homogeneus.xy = (homogeneus.xy + 1) * 0.5f;
    homogeneus.y = 1 - homogeneus.y;
    
    return homogeneus;

}