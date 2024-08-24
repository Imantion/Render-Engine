#include "..\declarations.hlsli"

struct GSOutput
{
	float4 pos : SV_POSITION;
    float3 spacePosition : SPACEPOS;
    float3 normal : VERTEXNORMAL;
    uint objectID : OBJECTID;
};

struct VOut
{
    float3 spacePosition : WORLDPOS;
    float3 normal : NORMAL;
    uint objectID : OBJECTID;
};

float distanceIntensity(float value, float target, float fade)
{
    return min(1.0, abs(value - target) / fade);
}

float periodIntensity(float value, float period, float fade)
{
    float target = round(value / period) * period;
    return distanceIntensity(value, target, fade);
}

float wave(float3 pos, float waveInterval, float waveYSpeed, float waveThickness, uniform bool distort)
{
    if (distort)
    {
        const float WAVE_XZ_SPEED = 3.0;
        const float WAVE_DISTORTION_SIZE = 0.035;
        const float WAVE_OSCILLATING_TIME = 4;

        float distortionSign = abs(frac(g_time / WAVE_OSCILLATING_TIME) - 0.5) * 4 - 1;
        float2 distortion = sin(pos.xz / WAVE_DISTORTION_SIZE + g_time * WAVE_XZ_SPEED) * WAVE_DISTORTION_SIZE * distortionSign;
        pos.y -= (distortion.x + distortion.y);
    }

    pos.y -= g_time * waveYSpeed;

    float intensity = 1.0 - periodIntensity(pos.y, waveInterval, waveThickness);
    return intensity;
}

// Note: in HLSL global constant is marked with "static const".
// One "const" is not enough, because it will be considered to be a uniform from a constant buffer.
// In HLSL const means that the value can not be changed by the shader.
// Adding "static" to global variable means that it is not visible to application, so doesn't belong to a constant buffer.
// A local constant inside a function can be marked just with "const".
// "static" for locals preserves value during current shader thread execution.

static const float BLUE_WAVE_INTERVAL = 0.8;
static const float BLUE_WAVE_SPEED = 0.25;
static const float BLUE_WAVE_THICKNESS = 0.05;

static const float RED_WAVE_INTERVAL = 10;
static const float RED_WAVE_SPEED = 2;
static const float RED_WAVE_THICKNESS = 0.2;

[maxvertexcount(3)]
void main(
	triangle VOut input[3],
	inout TriangleStream< GSOutput > output
)
{
    float offset = 0.0;
    float3 centr = (input[0].spacePosition + input[1].spacePosition + input[2].spacePosition) / 3.0f;
    offset += float3(1, 1, 1) * 0.025 * wave(centr, BLUE_WAVE_INTERVAL, BLUE_WAVE_SPEED, BLUE_WAVE_THICKNESS, true);
    offset += float3(1, 1, 1) * 0.05 * wave(centr, RED_WAVE_INTERVAL, RED_WAVE_SPEED, RED_WAVE_THICKNESS, false);
    float3 normal = normalize(cross(input[1].spacePosition - input[0].spacePosition, input[2].spacePosition - input[0].spacePosition));
	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;

        
        element.spacePosition = input[i].spacePosition + normal * offset;
        element.pos = mul(float4(element.spacePosition,1.0f), viewProjection);
        element.normal = input[i].normal;
        element.objectID = input[i].objectID;
		output.Append(element);
	}
}