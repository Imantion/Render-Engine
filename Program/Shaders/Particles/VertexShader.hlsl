#include "..\declarations.hlsli"

cbuffer textureInfo : register(b5)
{
    float frameSize;
    float framesPerRow;
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

static const float2 uvOffset[4] =
{
    float2(0.0f, -1.0f), // Bottom-left
    float2(1.0f, -1.0f), // Bottom-right
    float2(0.0f, 0.0f), // Top-left
    float2(1.0f, 0.0f) // Top-right
};

static const float2 uv[4] =
{
    float2(0.0f, 0.0f), // Bottom-left
    float2(1.0f, 0.0f), // Bottom-right
    float2(0.0f, 1.0f), // Top-left
    float2(1.0f, 1.0f) // Top-right
};

float2 calculateUV(float2 inputUV, uint frameIndex)
{
    int2 atlasCoord =
    {
        fmod(frameIndex, 8.0f),
		frameIndex / 8.0f
    };

    float2 singleFrameTextureSize = 1.0.xx / 8.0f;

    return inputUV / 8.0f + atlasCoord * singleFrameTextureSize;
}

VSOut main(ParticleInstance input)
{
    VSOut output;
    
    float3 cameraDir = normalize(g_cameraPosition - input.position);
    float3 right = normalize(cross(float3(0.0f, 1.0f, 0.0f), cameraDir)); // Right vector
    float3 up = cross(cameraDir, right); // Up vector
    
    float2 cornerOffset = offsets[input.vertexID];
    
     // Apply rotation to the offset
    float cosTheta = cos(input.rotation);
    float sinTheta = sin(input.rotation);
    float2 rotatedOffset = float2(
        cornerOffset.x * cosTheta - cornerOffset.y * sinTheta,
        cornerOffset.x * sinTheta + cornerOffset.y * cosTheta
    );

    // Scale the offset by the particle size
    rotatedOffset *= input.size;
    
    output.worldPos = input.position + (right * rotatedOffset.x) + (up * rotatedOffset.y);
    output.position = mul(float4(output.worldPos, 1.0f), viewProjection);
    output.rgba = input.rgba;
    
    float2 uvCoordOffset = uvOffset[input.vertexID] * frameSize;
    output.uvThis = float2(input.frameIndex % framesPerRow * frameSize, 1.0f - uint(input.frameIndex / framesPerRow) * frameSize) + uvCoordOffset;
    output.uvNext = float2((input.frameIndex + 1) % framesPerRow * frameSize, 1.0f - uint((input.frameIndex + 1) / framesPerRow) * frameSize) + uvCoordOffset;
    
    //output.uvThis = calculateUV(uv[input.vertexID], input.frameIndex);
    //output.uvNext = calculateUV(uv[input.vertexID], input.frameIndex);
    
    output.time = input.frameFraction;

    return output;
}