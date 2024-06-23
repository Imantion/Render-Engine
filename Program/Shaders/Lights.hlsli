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

#if MAX_SPOT_LIGTS
static const int MAX_SL = MAX_SPOT_LIGTS;
#else
static const int MAX_SL = 5;
#endif

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

cbuffer LigtData : register(b0)
{
    DirectionalLight directionalLights[MAX_DL];
    PointLight pointLights[MAX_PL];
    SpotLight spotLights[MAX_SL];
    int dlSize;
    int plSize;
    int slSize;
}