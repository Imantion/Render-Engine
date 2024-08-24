cbuffer PerFace : register(b2)
{
    float3 frustrum[18];
};

struct GSInput
{
    float4 pos : SV_POSITION;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    uint renderTargetArrayIndex : SV_RenderTargetArrayIndex;
};

static const float2 uvs[3] = {float2(0,0),float2(2,0),float2(0,2)};

[maxvertexcount(18)]
void main(triangle GSInput input[3], inout TriangleStream<PSInput> outputStream)
{
    for (uint face = 0; face < 6; face++)
    {
        uint index = 3 * face;
        for (uint i = 0; i < 3; i++)
        {
            PSInput output;
            output.pos = input[i].pos;
            //output.normal = frustrum[index] + (input[i].pos.x + 1) * 0.5 * frustrum[index + 1] + (input[i].pos.y + 1) * 0.5 * frustrum[index + 2];
            output.normal = frustrum[index + i];
            output.renderTargetArrayIndex = face;
            outputStream.Append(output);
        }
        outputStream.RestartStrip();
    }
}