#pragma pack_matrix(row_major)

cbuffer lightViewProjections : register(b2)
{
    float4x4 lightViewProjection[6];
};

struct GSInput
{
    float3 worldPos : WORLDPOS;
    float2 tc : TC;
    float3 spherePosition : SPHEREPOS;
    float sphereRadius : SPHERERADIUS;
    float spherePreviousRadius : SPHEREPREVRADIUS;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float2 tc : TC;
    float3 spherePosition : SPHEREPOS;
    float sphereRadius : SPHERERADIUS;
    float spherePreviousRadius : SPHEREPREVRADIUS;
    uint renderTargetArrayIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(18)]
void main(triangle GSInput input[3], inout TriangleStream<PSInput> outputStream)
{
    for (uint face = 0; face < 6; face++)
    {
        uint index = 3 * face;
        for (uint i = 0; i < 3; i++)
        {
            PSInput output;
            output.pos = mul(float4(input[i].worldPos, 1.0f), lightViewProjection[face]);
            output.worldPos = input[i].worldPos;
            output.tc = input[i].tc;
            output.spherePosition = input[i].spherePosition;
            output.sphereRadius = input[i].sphereRadius;
            output.spherePreviousRadius = input[i].spherePreviousRadius;
            output.renderTargetArrayIndex = face;
            outputStream.Append(output);
        }
        outputStream.RestartStrip();
    }
}