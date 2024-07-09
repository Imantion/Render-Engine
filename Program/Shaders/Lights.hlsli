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

static const float3 ambient = float3(0.05f, 0.05f, 0.05f);
Texture2D flashlighTexture : register(t1);
SamplerState samplerstateFlash : register(s0);

struct DirectionalLight
{
    float3 color;
    float intensity;
    float3 direction;
    float padding; // Padding to align structure size
};

// PointLight structure
struct PointLight
{
    float3 color;
    float intensity;
    float3 position;
    int bindedObjectId; // -1 means no object is bound
};

// SpotLight structure
struct SpotLight
{
    float3 color;
    float intensity;
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

float3 SpotLightContribution(SpotLight spotLight, float3 normal, float3 position, float3 cameraPosition)
{
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    
    float3 directionToLight = position - spotLight.position;
    float distancSquaredToLight = dot(directionToLight, directionToLight);
    directionToLight = normalize(directionToLight);
        
    float cosAngle = dot(directionToLight, spotLight.direction);
    if (cosAngle < spotLight.cutoffAngle)
    {
        return spotLight.color * 0.00001f;
    }
    float3 viewDir = normalize(cameraPosition - position);
    float3 halfwayDir = normalize(viewDir - directionToLight);
        
    float diff = max(dot(normal, -directionToLight), 0.00001f);
    float spec = pow(max(dot(halfwayDir, normal), 0.00001f), 32);
       
    float I = 1 - (1 - cosAngle) / (1 - spotLight.cutoffAngle);
        
    //float3 maskPos = normalize(directionToLight - spotLights[i].direction);
    //float2 uv = float2(0.0f, 0.0f);
    //uv.x = (dot(maskPos, float3(1.0f, 0.0f, 0.0f)) + 1.0f) * 0.5f;
    //uv.y = (dot(maskPos, float3(0.0f, 1.0f, 0.0f)) + 1.0f) * 0.5f;
        
        
    finalColor += spotLight.color * ((diff + spec) * (spotLight.intensity / distancSquaredToLight) * I);
    
    return finalColor;
}

float3 FlashLight(SpotLight flashLight, float3 normal, float3 position, float3 cameraPosition)
{
    float3 finalColor = SpotLightContribution(flashLight, normal, position, cameraPosition);
    float4 proj = mul(float4(position, 1.0f), lightViewProjection);
    float2 uv = (proj.xy) / proj.w;
    float3 mask = flashlighTexture.Sample(samplerstateFlash, uv * 0.5 + 0.5).rgb;
    
    return finalColor * mask;
}

float3 PointLightContribution(PointLight pointLight, float3 normal, float3 position, float3 cameraPosition)
{
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    

    float3 directionToLight = position - pointLight.position;
    float distancSquaredToLight = dot(directionToLight, directionToLight);
    directionToLight = normalize(directionToLight);
        
    float3 viewDir = normalize(cameraPosition - position);
    float3 halfwayDir = normalize(viewDir - directionToLight);
        
    float diff = max(dot(normal, -directionToLight), 0.00001f);
    float spec = pow(max(dot(halfwayDir, normal), 0.00001f), 32);
        
    finalColor += pointLight.color * ((diff + spec) * (pointLight.intensity / distancSquaredToLight));
    
    return finalColor;
}

float3 DirectionalLightsContibution(DirectionalLight directionalLight, float3 normal, float3 position, float3 cameraPosition)
{
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    
    
        
    float3 viewDir = normalize(cameraPosition - position);
    float3 halfwayDir = normalize(viewDir - directionalLight.direction);
        
    float diff = max(dot(normal, -directionalLight.direction), 0.00001f);
    float spec = pow(max(dot(halfwayDir, normal), 0.00001f), 32);
        
    finalColor += directionalLight.color * ((diff + spec) * directionalLight.intensity);
    
    
    return finalColor;
}