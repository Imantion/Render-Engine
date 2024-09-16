#include "..\declarations.hlsli"

cbuffer textureInfo : register(b7)
{
    uint framesPerColumn;
    uint framesPerRow;
}
    

struct ParticleInstance
{
    float4 rgba : COLOR;
    float3 position : POSITION;
    float rotation : ROTATION; 
    float2 size : SIZE;
    float frameFraction : FRAMEFRACTION;
    uint frameIndex : FRAMEINDEX;
    uint vertexID : SV_VertexID;
};


struct VSOut
{
    float4 position : SV_Position;
    float3 worldPos : WORLD;
    float4 rgba : RGBA;
    float time : TIME;
    float2 uvThis : UV_THIS;
    float2 uvNext : UV_NEXT;
};

static const float2 offsets[4] =
{
    float2(-0.5f, -0.5f), // Bottom-left
    float2(0.5f, -0.5f), // Bottom-right
    float2(-0.5f, 0.5f), // Top-left
    float2(0.5f, 0.5f) // Top-right
};

// Frisvad with z == -1 problem avoidance
void basisFromDir(out float3 right, out float3 top, in float3 dir)
{
    float k = 1.0 / max(1.0 + dir.z, 0.00001);
    float a = dir.y * k;
    float b = dir.y * a;
    float c = -dir.x * a;
    right = float3(dir.z + b, c, -dir.x);
    top = float3(c, 1.0 - b, -dir.y);
}


VSOut main(ParticleInstance input)
{
    VSOut output;

    //float3 cameraDir = normalize(g_cameraPosition - input.position);
    //float3 right = normalize(cross(float3(0.0f, 1.0f, 0.0f), cameraDir));
    //float3 up = cross(cameraDir, right);
    
    float3 cameraDir = -g_viewMatrix._31_32_33;
    float3 right = -g_viewMatrix._11_12_13;
    float3 up = g_viewMatrix._21_22_23;
    
    float2 cornerOffset = offsets[input.vertexID];
    
    float cosTheta = cos(input.rotation);
    float sinTheta = sin(input.rotation);
    float2 rotatedOffset = float2(
        cornerOffset.x * cosTheta - cornerOffset.y * sinTheta,
        cornerOffset.x * sinTheta + cornerOffset.y * cosTheta
    );

    rotatedOffset *= input.size;
    
    output.worldPos = input.position + (right * rotatedOffset.x) + (up * rotatedOffset.y);
    output.position = mul(float4(output.worldPos, 1.0f), viewProjection);
    output.rgba = input.rgba;
    
    float2 frameSize = float2(1.0f / (float) framesPerRow, 1.0f / (float) framesPerColumn);
    float2 uvCoordOffset = offsets[input.vertexID] + 0.5f;
    output.uvThis = (float2(input.frameIndex % framesPerRow, uint(input.frameIndex / framesPerRow)) + uvCoordOffset) * frameSize;
    output.uvNext = (float2((input.frameIndex + 1) % framesPerRow, uint((input.frameIndex + 1) / framesPerRow)) + uvCoordOffset) * frameSize;
    
    output.time = input.frameFraction;

    return output;
}