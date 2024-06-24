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
    int dlSize;
    int plSize;
    int slSize;
}

float3 SpotLightContribution(float3 normal, float3 position, float3 cameraPosition)
{
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < slSize; ++i)
    {
        float3 directionToLight = position - spotLights[i].position;
        float distancSquaredToLight = dot(directionToLight, directionToLight);
        directionToLight = normalize(directionToLight);
        
        if (dot(directionToLight, spotLights[i].direction) < cos(spotLights[i].cutoffAngle))
        {
            finalColor += spotLights[i].color * 0.001f;
            continue;
        }       
        float3 viewDir = normalize(cameraPosition - position);
        float3 halfwayDir = normalize(viewDir - directionToLight);
        
        float diff = max(dot(normal, -directionToLight), 0.001f);
        float spec = pow(max(dot(halfwayDir, normal), 0.001f), 32);
        
        finalColor += spotLights[i].color * ((diff + spec) * (spotLights[i].intensity / distancSquaredToLight));
    }
    
    return finalColor;
}

float3 PointLightContribution(float3 normal,float3 position, float3 cameraPosition)
{
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < plSize; ++i)
    {
        float3 directionToLight = position - pointLights[i].position;
        float distancSquaredToLight = dot(directionToLight, directionToLight);
        directionToLight = normalize(directionToLight);
        
        float3 viewDir = normalize(cameraPosition - position);
        float3 halfwayDir = normalize(viewDir - directionToLight);
        
        float diff = max(dot(normal, -directionToLight), 0.001f);
        float spec = pow(max(dot(halfwayDir, normal), 0.001f), 32);
        
        finalColor += pointLights[i].color * ((diff + spec) * (pointLights[i].intensity / distancSquaredToLight));
    }
    
    return finalColor;
} 

float3 DirectionalLightsContibution(float3 normal, float3 position, float3 cameraPosition)
{
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < dlSize; ++i)
    {
        
        float3 viewDir = normalize(cameraPosition - position);
        float3 halfwayDir = normalize(viewDir - directionalLights[i].direction);
        
        float diff = max(dot(normal, -directionalLights[i].direction), 0.001f);
        float spec = pow(max(dot(halfwayDir, normal), 0.001f), 32);
        
        finalColor += directionalLights[i].color * ((diff + spec) * directionalLights[i].intensity);
    }
    
    return finalColor;
}